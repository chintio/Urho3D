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
    IDirect3D9* interface_; // 接口对象，Direct3DCreate9(D3D_SDK_VERSION)
    /// Direct3D device.
    IDirect3DDevice9* device_; // 设备对象，IDirect3D9::CreateDevice
    /// Default color surface.
    IDirect3DSurface9* defaultColorSurface_; // 设备缺省颜色表面
    /// Default depth-stencil surface.
    IDirect3DSurface9* defaultDepthStencilSurface_; // 设备缺省深度模板表面
    /// Frame query for flushing the GPU command queue.
    IDirect3DQuery9* frameQuery_;
    /// Adapter number.
    DWORD adapter_; // 物理显卡的序号，D3DADAPTER_DEFAULT
    /// Device type.
    D3DDEVTYPE deviceType_; // 设备类型，硬件设备D3DDEVTYPE_HAL、软件设备D3DDEVTYPE_REF
    /// Device capabilities.
    D3DCAPS9 deviceCaps_; // 设备性能，IDirect3D9::GetDeviceCaps
    /// Adapter identifier.
    D3DADAPTER_IDENTIFIER9 adapterIdentifier_; // 适配器（显卡）信息，IDirect3D9::GetAdapterIdentifier
    /// Direct3D presentation parameters.
    D3DPRESENT_PARAMETERS presentParams_; // 设备显示参数
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
    bool queryIssued_; // 发起查询
    /// sRGB mode in use.
    bool sRGBModes_[MAX_TEXTURE_UNITS];
    /// sRGB write flag.
    bool sRGBWrite_;
    /// Color surfaces in use.
    IDirect3DSurface9* colorSurfaces_[MAX_RENDERTARGETS]; // 第0个为屏幕表面
    /// Depth-stencil surface in use.
    IDirect3DSurface9* depthStencilSurface_;
    /// Blending enabled flag.
    DWORD blendEnable_; // 混合状态D3DRS_ALPHABLENDENABLE
    /// Source blend mode.
    D3DBLEND srcBlend_; // 混合源D3DRS_SRCBLEND，d3dSrcBlend
    /// Destination blend mode.
    D3DBLEND destBlend_; // 混合目标D3DRS_DESTBLEND，d3dDestBlend
    /// Blend operation.
    D3DBLENDOP blendOp_; // 混合操作D3DRS_BLENDOP，d3dBlendOp
    /// Vertex declarations.
    VertexDeclarationMap vertexDeclarations_; // 使用过的顶点描述
    /// Stream frequencies by vertex buffer.
    unsigned streamFrequencies_[MAX_VERTEX_STREAMS];
    /// Stream offsets by vertex buffer.
    unsigned streamOffsets_[MAX_VERTEX_STREAMS];
    /// Vertex declaration in use.
    VertexDeclaration* vertexDeclaration_; // 当前使用的顶点描述
    /// Shader programs.
    ShaderProgramMap shaderPrograms_;
    /// Shader program in use.
    ShaderProgram* shaderProgram_;

};

// System Memory（SM）：就是系统内存，存放在System Memory上的Resource是不能直接被GPU访问的，对于GPU来说是不可见的，这种Resource必须经过拷贝才能运用到GPU的渲染中。
// AGP Memory（AM）: 也叫Share System Memory或者Non - Local  Video Memory, 其实就是系统内存的一部分，只是这部分内存已经被Mapping为显存的一部分，不但CPU可以访问这部分内存，而且GPU也可以通过PCI - E / AGP总线透明访问这部分内存，就像显存一样，但GPU访问速度比显存慢，毕竟经过映射，而且还要走PCI - E总线进行读写。
// Video Memory（VM）: 显存，也叫做Local Video Memory, 只有GPU可以访问，CPU不可见也不能访问，速度最快。

// D3DPOOL：
// D3DPOOL_DEFAULT：一般存放在VM或AM中。由于在系统内存中没有备份，在Device Lost时需要释放后重新创建。（创建到D3DPOOL_DEFAULT中的纹理是不能被CPU LOCK的，除非是动态纹理。但创建在D3DPOOL_DEFAULT中的VB IB RENDERTARGET BACK BUFFERS可以被LOCK。当你用D3DPOOL_DEFAULT创建资源时，如果显存已经使用完毕，则托管资源会被换出显存来释放足够的空间。）
// D3DPOOL_MANAGED：和D3DPOOL_DEFAULT的区别在于D3DRuntime会在SM中为其创建备份，Device Lost后不需要处理，D3DRuntime会自动用备份通过显卡驱动重新创建出资源。（创建的时候被放置在SM，在GPU需要使用资源时D3D RUNTIME自动将数据拷贝到VM中去，当资源被GPU修改后，RUNTIME在必要时自动将其更新到SM中来，而在SM中修改后也会被UPDATE到VM去中。所以被CPU或者GPU频发修改的数据，一定不要使用托管类型，这样会产生非常昂贵的同步负担。）
// D3DPOOL_SYSTEMMEM：只存放于SM中，这种资源不能被GPU访问，Device Lost后不需要处理。
// D3DPOOL_SCRATCH：只存放于SM中，不同于D3DPOOL_SYSTEMMEM，这些资源不受图形设备的制约。所以，设备无法访问该类型内存池中的资源。但这些资源之间可以互相复制。（和D3DPOOL_SYSTEMMEM差别是使用D3DPOOL_SYSTEMMEM时，资源格式受限于Device性能，因为资源很可能会被更新到AM或VM中去供图形系统使用，但SCRATCH只受RUNTIME限制，所以这种资源无法被图形系统使用。 ）
// 在创建Resource时，对指定 D3DPOOL时有些限制，比如创建RenderTarget和DepthStencil只能标志为D3DPOOL_DEFAULT，被指定为dynamic的resource只能标志为D3DPOOL_DEFAULT或D3DPOOL_SYSTEMMEM，详细可参考D3D SDK 文档。
// 总结：对于动态资源（RenderTarget、DepthStencil、D3DUSAGE_DYNAMIC资源），就用D3DPOOL_DEFAULT（Device Lost后自助创建）。
//       对于静态资源（不需要经常修改或访问的数据，例如，地形、城市建筑），可以用D3DPOOL_DEFAULT或D3DPOOL_MANAGED（Device Lost后由托管直接从备份中恢复）。

// D3DUSAGE：
// 显存与非D3DUSAGE_DYNAMIC:创建缓存时，如果未使用标记 D3DUSAGE_DYNAMIC ，则称所创建的缓存为静态缓存(static buffer)。静态缓存一般被放置在显存中，以保证存储于其中的数据得到最高效的处理。然而，静态缓存是以牺牲对静态缓存读写操作的速度为代价的，这是因为CPU访问显存的速度本身就很慢。基于上述原因，我们用静态缓存来存储静态数据(那些不需要经常修改或访问的数据)。静态缓存必须在应用程序初始化时用几何体的数据进行填充。
// AGP与D3DUSAGE_DYNAMIC: 创建缓存时，如果使用了标记 D3DUSAGE_DYNAMIC ， 则称所创建的缓存为动态缓存(dynamic buffer)。动态缓存一般放置在 AGP(Accelerated Graphics Port) 存储区中，其内容可被迅速更新。动态缓存中数据的处理速度不像静态缓存那样快，这是因为在绘制之前数据必须传输到显存中。但动态缓存的突出优点是其更新速度相当快(快速的 CPU 写操作)。所以，如果您需要频繁更新缓存中的内容，该缓存应设置为动态的。
// 对显存和AGP存储区进行读操作非常慢。所以，如果需要在程序运行时读取几何数据，最好在系统内存中保留一份副本，然后在需要时对其进行读操作。

// LOCK
// 1，如果LOCK DEFAULT资源会发生什么情况呢？DEFAULT资源可能在VM或AM中，
//      a）如果在VM中，必须在系统内容中开辟一个临时缓冲返回给数据，当应用程序将数据填充到临时缓冲后，UNLOCK的时候，RUNTIME会将临时缓冲的数据传回到VM中去，如果资源D3DUSAGE属性不是WRITEONLY的，则系统还需要先从VM里拷贝一份原始数据到临时缓冲区，这就是为什么不指定WRITEONLY会降低程序性能的原因。
//      b）CPU写AM也有需要注意的地方，因为CPU写AM一般是WRITE COMBINING，也就是说将写缓冲到一个CACHE LINE上，当CACHE LINE满了之后才FLUSH到AM中去，第一个要注意的就是写数据必须是WEAK ORDER的（图形数据一般都满足这个要求），第二请尽量一次写满一个CACHE LINE，否则会有额外延迟，因为CPU每次必须FLUSH整个CACHE LINE到目标，但如果我们只写了LINE中部分字节，CPU必须先从AM中读取整个LINE长数据COMBINE后重新FLUSH。第三尽可能顺序写，随机写会让WRITE COMBINING反而变成累赘，如果是随机写资源，不要使用D3DUSAGE_DYNAMIC创建，请使用D3DPOOL_MANAGED，这样写会完全在SM中完成。
// 2，普通纹理（D3DPOOL_DEFAULT）是不能被锁定的，因为其位于VM中，只能通过UPDATESURFACE和UPDATETEXTURE（IDirect3DDevice9::UpdateSurface or IDirect3DDevice9::UpdateTexture）来访问，为什么D3D不让我们锁定静态纹理，却让我们锁定静态VB IB呢？我猜测可能有2个方面的原因，
//      a）第一就是纹理矩阵一般十分庞大，且纹理在GPU内部已二维方式存储；
//      b）第二是纹理在GPU内部是以NATIVE FORMAT方式存储的，并不是明文enRGBA格式。动态纹理因为表明这个纹理需要经常修改，所以D3D会特别存储对待，
// 3，高频率修改的动态纹理不适合用动态属性创建，在此分两种情况说明，一种是GPU写入的RENDERTARGET，一种是CPU写入的TEXTURE VIDEO，我们知道动态资源一般是放置在AM中的，GPU访问AM需要经过AGP / PCI - E总线，速度较VM慢许多，而CPU访问AM又较SM慢很多，如果资源为动态属性，意味着GPU和CPU访问资源会持续的延迟，所以此类资源最好以D3DPOOL_DEFAULT和D3DPOOL_SYSTEMMEM各创建一份，自己手动进行双向更新更好。
// 4，千万别 RENDERTARGET以D3DPOOL_MANAGED 属性创建，这样效率极低，原因自己分析。而对于改动不太频繁的资源则推荐使用DEFAULT创建，自己手动更新，因为一次更新的效率损失远比GPU持续访问AM带来的损失要小。
// 5，不合理的LOCK会严重影响程序性能，因为一般LOCK需要等待COMMAND BUFFER前面的绘制指令全部执行完毕才能返回，否则很可能修改正在使用的资源，从LOCK返回到修改完毕UNLOCK这段时间GPU全部处于空闲状态，没有合理使用GPU和CPU的并行性，DX8.0引进了一个新的LOCK标志D3DLOCK_DISCARD，表示不会读取资源，只会全写资源，这样驱动和RUNTIME配合来了个瞒天过海，立即返回给应用程序另外块VM地址指针，而原指针在本次UNLOCK之后被丢弃不再使用，这样CPU LOCK无需等待GPU使用资源完毕，能继续操作图形资源（顶点缓冲和索引缓冲），这技术叫VB IB换名（renaming）。

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

// https://user.xmission.com/~legalize/book/preview/poster/pipeline-9.0.png
// 对于D3DPOOL，这篇文章讲的非常清楚 https://blog.csdn.net/poem_of_sunshine/article/details/8619996

}
