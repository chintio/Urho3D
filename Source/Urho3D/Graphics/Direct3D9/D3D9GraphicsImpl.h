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

#pragma once

#include "../../Graphics/ShaderProgram.h"
#include "../../Graphics/VertexDeclaration.h"
#include "../../Math/Color.h"

#include <d3d9.h>

namespace Urho3D
{

#define URHO3D_SAFE_RELEASE(p) if (p) { ((IUnknown*)p)->Release();  p = 0; }

#define URHO3D_LOGD3DERROR(msg, hr) URHO3D_LOGERRORF("%s (HRESULT %x)", msg, (unsigned)hr)

using ShaderProgramMap = HashMap<Pair<ShaderVariation*, ShaderVariation*>, SharedPtr<ShaderProgram> >;
using VertexDeclarationMap = HashMap<unsigned long long, SharedPtr<VertexDeclaration> >;

/// %Graphics implementation. Holds API-specific objects.
class URHO3D_API GraphicsImpl
{
    friend class Graphics;

public:
    /// Construct.
    GraphicsImpl();

    /// Return Direct3D device.
    IDirect3DDevice9* GetDevice() const { return device_; }

    /// Return device capabilities.
    const D3DCAPS9& GetDeviceCaps() const { return deviceCaps_; }

    /// Return adapter identifier.
    const D3DADAPTER_IDENTIFIER9& GetAdapterIdentifier() const { return adapterIdentifier_; }

    /// Return whether a texture format and usage is supported.
    bool CheckFormatSupport(D3DFORMAT format, DWORD usage, D3DRESOURCETYPE type);

    /// Return whether a multisample level is supported.
    bool CheckMultiSampleSupport(D3DFORMAT format, int level);

private:
    /// Direct3D interface.
    IDirect3D9* interface_;
    /// Direct3D device.
    IDirect3DDevice9* device_;
    /// Default color surface.
    IDirect3DSurface9* defaultColorSurface_;
    /// Default depth-stencil surface.
    IDirect3DSurface9* defaultDepthStencilSurface_;
    /// Frame query for flushing the GPU command queue.
    IDirect3DQuery9* frameQuery_;
    /// Adapter number.
    DWORD adapter_;
    /// Device type.
    D3DDEVTYPE deviceType_;
    /// Device capabilities.
    D3DCAPS9 deviceCaps_;
    /// Adapter identifier.
    D3DADAPTER_IDENTIFIER9 adapterIdentifier_;
    /// Direct3D presentation parameters.
    D3DPRESENT_PARAMETERS presentParams_;
    /// Texture min filter modes in use.
    D3DTEXTUREFILTERTYPE minFilters_[MAX_TEXTURE_UNITS];
    /// Texture mag filter modes in use.
    D3DTEXTUREFILTERTYPE magFilters_[MAX_TEXTURE_UNITS];
    /// Texture mip filter modes in use.
    D3DTEXTUREFILTERTYPE mipFilters_[MAX_TEXTURE_UNITS];
    /// Texture U coordinate addressing modes in use.
    D3DTEXTUREADDRESS uAddressModes_[MAX_TEXTURE_UNITS];
    /// Texture V coordinate addressing modes in use.
    D3DTEXTUREADDRESS vAddressModes_[MAX_TEXTURE_UNITS];
    /// Texture W coordinate addressing modes in use.
    D3DTEXTUREADDRESS wAddressModes_[MAX_TEXTURE_UNITS];
    /// Texture anisotropy setting in use.
    unsigned maxAnisotropy_[MAX_TEXTURE_UNITS];
    /// Texture border colors in use.
    Color borderColors_[MAX_TEXTURE_UNITS];
    /// Device lost flag.
    bool deviceLost_;
    /// Frame query issued flag.
    bool queryIssued_;
    /// sRGB mode in use.
    bool sRGBModes_[MAX_TEXTURE_UNITS];
    /// sRGB write flag.
    bool sRGBWrite_;
    /// Color surfaces in use.
    IDirect3DSurface9* colorSurfaces_[MAX_RENDERTARGETS];
    /// Depth-stencil surface in use.
    IDirect3DSurface9* depthStencilSurface_;
    /// Blending enabled flag.
    DWORD blendEnable_;
    /// Source blend mode.
    D3DBLEND srcBlend_;
    /// Destination blend mode.
    D3DBLEND destBlend_;
    /// Blend operation.
    D3DBLENDOP blendOp_;
    /// Vertex declarations.
    VertexDeclarationMap vertexDeclarations_;
    /// Stream frequencies by vertex buffer.
    unsigned streamFrequencies_[MAX_VERTEX_STREAMS];
    /// Stream offsets by vertex buffer.
    unsigned streamOffsets_[MAX_VERTEX_STREAMS];
    /// Vertex declaration in use.
    VertexDeclaration* vertexDeclaration_;
    /// Shader programs.
    ShaderProgramMap shaderPrograms_;
    /// Shader program in use.
    ShaderProgram* shaderProgram_;

};

// System Memory：就是系统内存，存放在System Memory上的Resource是不能直接被GPU访问的，对于GPU来说是不可见的，这种Resource必须经过拷贝才能运用到GPU的渲染中。
// AGP Memory : 也叫Share System Memory或者Non - Local  Video Memory, 其实就是系统内存的一部分，只是这部分内存已经被Mapping为显存的一部分，不但CPU可以访问这部分内存，而且GPU也可以通过PCI - E / AGP总线透明访问这部分内存，就像显存一样，但GPU访问速度比显存慢，毕竟经过映射，而且还要走PCI - E总线进行读写。
// Video Memory : 显存，也叫做Local Video Memory, 只有GPU可以访问，CPU不可见也不能访问，速度最快。

// D3DPOOL：
// D3DPOOL_DEFAULT：一般存放在video memory或AGP memory中。由于在系统内存中没有备份，在Device Lost时需要释放后重新创建。
// D3DPOOL_MANAGED：和D3DPOOL_DEFAULT的区别在于D3DRuntime会在System Memory中为其创建备份，Device Lost后不需要处理，D3DRuntime会自动用备份通过显卡驱动重新创建出资源。
// D3DPOOL_SYSTEMMEM：只存放于系统内存中，这种资源不能被GPU访问，Device Lost后不需要处理。
// D3DPOOL_SCRATCH：只存放于系统内存中，不同于D3DPOOL_SYSTEMMEM，这些资源不受图形设备的制约。所以，设备无法访问该类型内存池中的资源。但这些资源之间可以互相复制。
// 在创建Resource时，对指定 D3DPOOL时有些限制，比如创建RenderTarget和DepthStencil只能标志为D3DPOOL_DEFAULT，被指定为dynamic的resource只能标志为D3DPOOL_DEFAULT或D3DPOOL_SYSTEMMEM，详细可参考D3D SDK 文档。
// 总结：对于动态资源（RenderTarget、DepthStencil、D3DUSAGE_DYNAMIC资源），就用D3DPOOL_DEFAULT（Device Lost后自助创建）。
//       对于静态资源（不需要经常修改或访问的数据，例如，地形、城市建筑），可以用D3DPOOL_DEFAULT或D3DPOOL_MANAGED（Device Lost后由托管直接从备份中恢复）。

// D3DUSAGE：
// 显存与非D3DUSAGE_DYNAMIC:创建缓存时，如果未使用标记 D3DUSAGE_DYNAMIC ，则称所创建的缓存为静态缓存(static buffer)。静态缓存一般被放置在显存中，以保证存储于其中的数据得到最高效的处理。然而，静态缓存是以牺牲对静态缓存读写操作的速度为代价的，这是因为访问显存的速度本身就很慢。基于上述原因，我们用静态缓存来存储静态数据(那些不需要经常修改或访问的数据)。静态缓存必须在应用程序初始化时用几何体的数据进行填充。
// AGP与D3DUSAGE_DYNAMIC: 创建缓存时，如果使用了标记 D3DUSAGE_DYNAMIC ， 则称所创建的缓存为动态缓存(dynamic buffer)。动态缓存一般放置在 AGP(Accelerated Graphics Port) 存储区中，其内容可被迅速更新。动态缓存中数据的处理速度不像静态缓存那样快，这是因为在绘制之前数据必须传输到显存中。但动态缓存的突出优点是其更新速度相当快(快速的 CPU 写操作)。所以，如果您需要频繁更新缓存中的内容，该缓存应设置为动态的。
// 对显存和AGP存储区进行读操作非常慢。所以，如果需要在程序运行时读取几何数据，最好在系统内存中保留一份副本，然后在需要时对其进行读操作。

// 顶点着色器：
// 顶点着色器取得固定流水线中的变换和光照环节，就是顶点从局部坐标到齐次坐标的过程，包括世界变换、观察变换、投影变换。
// 顶点声明中的元素（Device->SetVertexDeclaration(D3DVERTEXELEMENT9)）将映射到顶点着色器的输入元素。
// 顶点着色器的输出包括：位置、顶点的点尺寸、雾融合值、顶点颜色、顶点纹理坐标。

// 像素着色器：
// 像素着色器是在对每个像素进行光栅化处理期间运行在图形卡上的一段程序。像素着色器实质上是取代了固定流水线中的多重纹理环节。
// 像素着色器的输入包括每个像素颜色和纹理坐标。在进入像素着色器之前，Direct3D先根据顶点颜色和顶点纹理坐标计算出每个像素的颜色和纹理坐标。输入像素着色器的颜色和纹理坐标对的个数由顶点着色器输出的颜色和纹理坐标对的个数决定。
// 就输出而言，像素着色器将输出计算所得的每个像素的单个颜色值。

// 手法（technique）与路径（pass）
// 一个效果文件中包含了一种或多种手法。
//  例如，我们可能想实现某种效果的两个版本，一种用着色器实现，另一种用固定功能流水线实现。按照这种方式，如果用户的图形卡支持着色器，就可以利用着色器，其余情况则使用固定功能流水线实现。
// 每种手法都包含了一条或多条路径。路径封装了设备状态、采样器以及（或）用于为该条特定路径绘制几何体的着色器。使用多条路径的原因是由于要想实现某些特效，必须对每条路径，以不同的绘制状态和着色器将同一几何体进行多次绘制。
//  例如，为了获得镜面反射效果，我们在每帧图像中都用不同的额绘制状态对相同的几何体进行了多次绘制。

}
