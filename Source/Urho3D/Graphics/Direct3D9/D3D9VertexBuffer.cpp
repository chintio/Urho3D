//
// Copyright (c) 2008-2019 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "../../Precompiled.h"

#include "../../Graphics/Graphics.h"
#include "../../Graphics/GraphicsImpl.h"
#include "../../Graphics/VertexBuffer.h"
#include "../../IO/Log.h"

#include "../../DebugNew.h"

namespace Urho3D
{

// 设备丢失，如果是动态数据，则需要释放对象，便于后续重建
void VertexBuffer::OnDeviceLost()
{
    // Dynamic buffers are in the default pool and need to be released on device loss
    if (dynamic_)
        Release();
}

// 恢复设备对象，将内存数据同步到设备缓存
void VertexBuffer::OnDeviceReset()
{
    // Dynamic buffers are in the default pool and need to be recreated after device reset
    if (dynamic_ || !object_.ptr_)
    {
        Create();
        dataLost_ = !UpdateToGPU();
    }
    else if (dataPending_)
        dataLost_ = !UpdateToGPU();

    dataPending_ = false;
}

// 释放设备对象
void VertexBuffer::Release()
{
    Unlock(); // 数据同步到设备

    // 删除graphics_中的引用项
    if (graphics_)
    {
        for (unsigned i = 0; i < MAX_VERTEX_STREAMS; ++i)
        {
            if (graphics_->GetVertexBuffer(i) == this)
                graphics_->SetVertexBuffer(nullptr);
        }
    }

    // 释放设备对象
    URHO3D_SAFE_RELEASE(object_.ptr_);
}

// 将数据更新到GPU，且同步到影子缓冲区
bool VertexBuffer::SetData(const void* data)
{
    if (!data)
    {
        URHO3D_LOGERROR("Null pointer for vertex buffer data");
        return false;
    }

    if (!vertexSize_)
    {
        URHO3D_LOGERROR("Vertex elements not defined, can not set vertex buffer data");
        return false;
    }

    if (shadowData_ && data != shadowData_.Get()) // 需要复制到影子缓冲区
        memcpy(shadowData_.Get(), data, vertexCount_ * vertexSize_);

    if (object_.ptr_)
    {
        if (graphics_->IsDeviceLost())
        {
            URHO3D_LOGWARNING("Vertex buffer data assignment while device is lost");
            dataPending_ = true;
            return true;
        }

        void* hwData = MapBuffer(0, vertexCount_, true);
        if (hwData)
        {
            memcpy(hwData, data, vertexCount_ * vertexSize_);
            UnmapBuffer();
        }
        else
            return false;
    }

    dataLost_ = false;
    return true;
}

// 将指定范围的数据更新到GPU，且同步到影子缓冲区
bool VertexBuffer::SetDataRange(const void* data, unsigned start, unsigned count, bool discard)
{
    if (start == 0 && count == vertexCount_) // 更新整个区域
        return SetData(data);

    if (!data)
    {
        URHO3D_LOGERROR("Null pointer for vertex buffer data");
        return false;
    }

    if (!vertexSize_)
    {
        URHO3D_LOGERROR("Vertex elements not defined, can not set vertex buffer data");
        return false;
    }

    if (start + count > vertexCount_)
    {
        URHO3D_LOGERROR("Illegal range for setting new vertex buffer data");
        return false;
    }

    if (!count)
        return true;

    if (shadowData_ && shadowData_.Get() + start * vertexSize_ != data) // 需要同步到影子缓存
        memcpy(shadowData_.Get() + start * vertexSize_, data, count * vertexSize_);

	// 更新到GPU内存
    if (object_.ptr_)
    {
        if (graphics_->IsDeviceLost())
        {
            URHO3D_LOGWARNING("Vertex buffer data assignment while device is lost");
            dataPending_ = true;
            return true;
        }

        void* hwData = MapBuffer(start, count, discard); // 将设备缓存映射到系统缓存
        if (hwData)
        {
            memcpy(hwData, data, count * vertexSize_); // 拷贝数据到映射的系统缓存，
            UnmapBuffer(); // 将映射的数据同步到设备缓存
        }
        else
            return false;
    }

    return true;
}

// 返回对应的缓冲区（GPU内存、影子缓存、或graphics_管理的系统内存）
void* VertexBuffer::Lock(unsigned start, unsigned count, bool discard)
{
    if (lockState_ != LOCK_NONE) // 已经被锁定（Lock、Unlock未配对）
    {
        URHO3D_LOGERROR("Vertex buffer already locked");
        return nullptr;
    }

    if (!vertexSize_) // 顶点大小未定义
    {
        URHO3D_LOGERROR("Vertex elements not defined, can not lock vertex buffer");
        return nullptr;
    }

    if (start + count > vertexCount_) // 锁定区域超过范围
    {
        URHO3D_LOGERROR("Illegal range for locking vertex buffer");
        return nullptr;
    }

    if (!count)
        return nullptr;

    lockStart_ = start;
    lockCount_ = count;

    // Because shadow data must be kept in sync, can only lock hardware buffer if not shadowed
    if (object_.ptr_ && !shadowData_ && !graphics_->IsDeviceLost()) // 只有设备数据可用时，则返回设备数据（IDirect3DVertexBuffer9）
        return MapBuffer(start, count, discard);
    else if (shadowData_) // 返回影子数据缓存
    {
        lockState_ = LOCK_SHADOW;
        return shadowData_.Get() + start * vertexSize_;
    }
    else if (graphics_) // 返回graphics_管理的系统内存
    {
        lockState_ = LOCK_SCRATCH;
        lockScratchData_ = graphics_->ReserveScratchBuffer(count * vertexSize_);
        return lockScratchData_;
    }
    else
        return nullptr;
}

// 数据填入GPU缓存
void VertexBuffer::Unlock()
{
    switch (lockState_)
    {
    case LOCK_HARDWARE: // 将数据从设备缓存的内存映射同步到设备缓存
        UnmapBuffer();
        break;

    case LOCK_SHADOW: // 将数据从影子缓存同步到设备缓存
        SetDataRange(shadowData_.Get() + lockStart_ * vertexSize_, lockStart_, lockCount_);
        lockState_ = LOCK_NONE;
        break;

    case LOCK_SCRATCH: // 将数据从graphics_管理的系统内存同步到影子缓存和设备缓存
        SetDataRange(lockScratchData_, lockStart_, lockCount_);
        if (graphics_)
            graphics_->FreeScratchBuffer(lockScratchData_);
        lockScratchData_ = nullptr;
        lockState_ = LOCK_NONE;
        break;

    default: break;
    }
}

// 创建设备顶点缓冲区IDirect3DVertexBuffer9，动态数据使用D3DPOOL_DEFAULT、D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY，否则使用D3DPOOL_MANAGED、0
bool VertexBuffer::Create()
{
    Release();

    if (!vertexCount_ || elements_.Empty())
        return true;

    if (graphics_)
    {
        if (graphics_->IsDeviceLost())
        {
            URHO3D_LOGWARNING("Vertex buffer creation while device is lost");
            return true;
        }

        unsigned pool = dynamic_ ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
        unsigned d3dUsage = dynamic_ ? D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY : 0;

        IDirect3DDevice9* device = graphics_->GetImpl()->GetDevice();
        HRESULT hr = device->CreateVertexBuffer(
            vertexCount_ * vertexSize_,
            d3dUsage,
            0,
            (D3DPOOL)pool,
            (IDirect3DVertexBuffer9**)&object_.ptr_,
            nullptr);
        if (FAILED(hr))
        {
            URHO3D_SAFE_RELEASE(object_.ptr_);
            URHO3D_LOGD3DERROR("Could not create vertex buffer", hr);
            return false;
        }
    }

    return true;
}

// 将影子数据更新到GPU
bool VertexBuffer::UpdateToGPU()
{
    if (object_.ptr_ && shadowData_)
        return SetData(shadowData_.Get());
    else
        return false;
}

// GPU缓存映射到系统缓存，供CPU访问(IDirect3DVertexBuffer9::Lock)
void* VertexBuffer::MapBuffer(unsigned start, unsigned count, bool discard)
{
    void* hwData = nullptr;

    if (object_.ptr_)
    {
        DWORD flags = 0;

        // 不合理的LOCK会严重影响程序性能，因为一般LOCK需要等待COMMAND BUFFER前面的绘制指令全部
        // 执行完毕才能返回，否则很可能修改正在使用的资源，从LOCK返回到修改完毕UNLOCK这段时间GPU
        // 全部处于空闲状态，没有合理使用GPU和CPU的并行性，DX8.0引进了一个新的LOCK标志D3DLOCK_DISCARD，
        // 表示不会读取资源，只会全写资源，这样驱动和RUNTIME配合来了个瞒天过海，立即返回给应用程序
        // 另外块VM地址指针，而原指针在本次UNLOCK之后被丢弃不再使用，这样CPU LOCK无需等待GPU使用
        // 资源完毕，能继续操作图形资源（顶点缓冲和索引缓冲），这技术叫VB IB换名（renaming）。
        if (discard && dynamic_)
            flags = D3DLOCK_DISCARD;

        HRESULT hr = ((IDirect3DVertexBuffer9*)object_.ptr_)->Lock(start * vertexSize_, count * vertexSize_, &hwData, flags);
        if (FAILED(hr))
            URHO3D_LOGD3DERROR("Could not lock vertex buffer", hr);
        else
            lockState_ = LOCK_HARDWARE;
    }

    return hwData;
}

// 将映射的系统缓存数据同步到设备缓存（IDirect3DVertexBuffer9::UnLock）
void VertexBuffer::UnmapBuffer()
{
    if (object_.ptr_ && lockState_ == LOCK_HARDWARE)
    {
        ((IDirect3DVertexBuffer9*)object_.ptr_)->Unlock();
        lockState_ = LOCK_NONE;
    }
}

}
