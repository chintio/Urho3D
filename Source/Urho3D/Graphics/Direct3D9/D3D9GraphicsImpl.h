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

// System Memory������ϵͳ�ڴ棬�����System Memory�ϵ�Resource�ǲ���ֱ�ӱ�GPU���ʵģ�����GPU��˵�ǲ��ɼ��ģ�����Resource���뾭�������������õ�GPU����Ⱦ�С�
// AGP Memory : Ҳ��Share System Memory����Non - Local  Video Memory, ��ʵ����ϵͳ�ڴ��һ���֣�ֻ���ⲿ���ڴ��Ѿ���MappingΪ�Դ��һ���֣�����CPU���Է����ⲿ���ڴ棬����GPUҲ����ͨ��PCI - E / AGP����͸�������ⲿ���ڴ棬�����Դ�һ������GPU�����ٶȱ��Դ������Ͼ�����ӳ�䣬���һ�Ҫ��PCI - E���߽��ж�д��
// Video Memory : �Դ棬Ҳ����Local Video Memory, ֻ��GPU���Է��ʣ�CPU���ɼ�Ҳ���ܷ��ʣ��ٶ���졣

// D3DPOOL��
// D3DPOOL_DEFAULT��һ������video memory��AGP memory�С�������ϵͳ�ڴ���û�б��ݣ���Device Lostʱ��Ҫ�ͷź����´�����
// D3DPOOL_MANAGED����D3DPOOL_DEFAULT����������D3DRuntime����System Memory��Ϊ�䴴�����ݣ�Device Lost����Ҫ����D3DRuntime���Զ��ñ���ͨ���Կ��������´�������Դ��
// D3DPOOL_SYSTEMMEM��ֻ�����ϵͳ�ڴ��У�������Դ���ܱ�GPU���ʣ�Device Lost����Ҫ����
// D3DPOOL_SCRATCH��ֻ�����ϵͳ�ڴ��У���ͬ��D3DPOOL_SYSTEMMEM����Щ��Դ����ͼ���豸����Լ�����ԣ��豸�޷����ʸ������ڴ���е���Դ������Щ��Դ֮����Ի��ิ�ơ�
// �ڴ���Resourceʱ����ָ�� D3DPOOLʱ��Щ���ƣ����紴��RenderTarget��DepthStencilֻ�ܱ�־ΪD3DPOOL_DEFAULT����ָ��Ϊdynamic��resourceֻ�ܱ�־ΪD3DPOOL_DEFAULT��D3DPOOL_SYSTEMMEM����ϸ�ɲο�D3D SDK �ĵ���
// �ܽ᣺���ڶ�̬��Դ��RenderTarget��DepthStencil��D3DUSAGE_DYNAMIC��Դ��������D3DPOOL_DEFAULT��Device Lost��������������
//       ���ھ�̬��Դ������Ҫ�����޸Ļ���ʵ����ݣ����磬���Ρ����н�������������D3DPOOL_DEFAULT��D3DPOOL_MANAGED��Device Lost�����й�ֱ�Ӵӱ����лָ�����

// D3DUSAGE��
// �Դ����D3DUSAGE_DYNAMIC:��������ʱ�����δʹ�ñ�� D3DUSAGE_DYNAMIC ������������Ļ���Ϊ��̬����(static buffer)����̬����һ�㱻�������Դ��У��Ա�֤�洢�����е����ݵõ����Ч�Ĵ���Ȼ������̬�������������Ծ�̬�����д�������ٶ�Ϊ���۵ģ�������Ϊ�����Դ���ٶȱ���ͺ�������������ԭ�������þ�̬�������洢��̬����(��Щ����Ҫ�����޸Ļ���ʵ�����)����̬���������Ӧ�ó����ʼ��ʱ�ü���������ݽ�����䡣
// AGP��D3DUSAGE_DYNAMIC: ��������ʱ�����ʹ���˱�� D3DUSAGE_DYNAMIC �� ����������Ļ���Ϊ��̬����(dynamic buffer)����̬����һ������� AGP(Accelerated Graphics Port) �洢���У������ݿɱ�Ѹ�ٸ��¡���̬���������ݵĴ����ٶȲ���̬���������죬������Ϊ�ڻ���֮ǰ���ݱ��봫�䵽�Դ��С�����̬�����ͻ���ŵ���������ٶ��൱��(���ٵ� CPU д����)�����ԣ��������ҪƵ�����»����е����ݣ��û���Ӧ����Ϊ��̬�ġ�
// ���Դ��AGP�洢�����ж������ǳ��������ԣ������Ҫ�ڳ�������ʱ��ȡ�������ݣ������ϵͳ�ڴ��б���һ�ݸ�����Ȼ������Ҫʱ������ж�������

// ������ɫ����
// ������ɫ��ȡ�ù̶���ˮ���еı任�͹��ջ��ڣ����Ƕ���Ӿֲ����굽�������Ĺ��̣���������任���۲�任��ͶӰ�任��
// ���������е�Ԫ�أ�Device->SetVertexDeclaration(D3DVERTEXELEMENT9)����ӳ�䵽������ɫ��������Ԫ�ء�
// ������ɫ�������������λ�á�����ĵ�ߴ硢���ں�ֵ��������ɫ�������������ꡣ

// ������ɫ����
// ������ɫ�����ڶ�ÿ�����ؽ��й�դ�������ڼ�������ͼ�ο��ϵ�һ�γ���������ɫ��ʵ������ȡ���˹̶���ˮ���еĶ��������ڡ�
// ������ɫ�����������ÿ��������ɫ���������ꡣ�ڽ���������ɫ��֮ǰ��Direct3D�ȸ��ݶ�����ɫ�Ͷ���������������ÿ�����ص���ɫ���������ꡣ����������ɫ������ɫ����������Եĸ����ɶ�����ɫ���������ɫ����������Եĸ���������
// ��������ԣ�������ɫ��������������õ�ÿ�����صĵ�����ɫֵ��

// �ַ���technique����·����pass��
// һ��Ч���ļ��а�����һ�ֻ�����ַ���
//  ���磬���ǿ�����ʵ��ĳ��Ч���������汾��һ������ɫ��ʵ�֣���һ���ù̶�������ˮ��ʵ�֡��������ַ�ʽ������û���ͼ�ο�֧����ɫ�����Ϳ���������ɫ�������������ʹ�ù̶�������ˮ��ʵ�֡�
// ÿ���ַ���������һ�������·����·����װ���豸״̬���������Լ���������Ϊ�����ض�·�����Ƽ��������ɫ����ʹ�ö���·����ԭ��������Ҫ��ʵ��ĳЩ��Ч�������ÿ��·�����Բ�ͬ�Ļ���״̬����ɫ����ͬһ��������ж�λ��ơ�
//  ���磬Ϊ�˻�þ��淴��Ч����������ÿ֡ͼ���ж��ò�ͬ�Ķ����״̬����ͬ�ļ���������˶�λ��ơ�

}
