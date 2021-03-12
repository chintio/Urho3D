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
    IDirect3D9* interface_; // �ӿڶ���Direct3DCreate9(D3D_SDK_VERSION)
    /// Direct3D device.
    IDirect3DDevice9* device_; // �豸����IDirect3D9::CreateDevice
    /// Default color surface.
    IDirect3DSurface9* defaultColorSurface_; // �豸ȱʡ��ɫ����
    /// Default depth-stencil surface.
    IDirect3DSurface9* defaultDepthStencilSurface_; // �豸ȱʡ���ģ�����
    /// Frame query for flushing the GPU command queue.
    IDirect3DQuery9* frameQuery_;
    /// Adapter number.
    DWORD adapter_; // �����Կ�����ţ�D3DADAPTER_DEFAULT
    /// Device type.
    D3DDEVTYPE deviceType_; // �豸���ͣ�Ӳ���豸D3DDEVTYPE_HAL������豸D3DDEVTYPE_REF
    /// Device capabilities.
    D3DCAPS9 deviceCaps_; // �豸���ܣ�IDirect3D9::GetDeviceCaps
    /// Adapter identifier.
    D3DADAPTER_IDENTIFIER9 adapterIdentifier_; // ���������Կ�����Ϣ��IDirect3D9::GetAdapterIdentifier
    /// Direct3D presentation parameters.
    D3DPRESENT_PARAMETERS presentParams_; // �豸��ʾ����
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
    bool queryIssued_; // �����ѯ
    /// sRGB mode in use.
    bool sRGBModes_[MAX_TEXTURE_UNITS];
    /// sRGB write flag.
    bool sRGBWrite_;
    /// Color surfaces in use.
    IDirect3DSurface9* colorSurfaces_[MAX_RENDERTARGETS]; // ��0��Ϊ��Ļ����
    /// Depth-stencil surface in use.
    IDirect3DSurface9* depthStencilSurface_;
    /// Blending enabled flag.
    DWORD blendEnable_; // ���״̬D3DRS_ALPHABLENDENABLE
    /// Source blend mode.
    D3DBLEND srcBlend_; // ���ԴD3DRS_SRCBLEND��d3dSrcBlend
    /// Destination blend mode.
    D3DBLEND destBlend_; // ���Ŀ��D3DRS_DESTBLEND��d3dDestBlend
    /// Blend operation.
    D3DBLENDOP blendOp_; // ��ϲ���D3DRS_BLENDOP��d3dBlendOp
    /// Vertex declarations.
    VertexDeclarationMap vertexDeclarations_; // ʹ�ù��Ķ�������
    /// Stream frequencies by vertex buffer.
    unsigned streamFrequencies_[MAX_VERTEX_STREAMS];
    /// Stream offsets by vertex buffer.
    unsigned streamOffsets_[MAX_VERTEX_STREAMS];
    /// Vertex declaration in use.
    VertexDeclaration* vertexDeclaration_; // ��ǰʹ�õĶ�������
    /// Shader programs.
    ShaderProgramMap shaderPrograms_;
    /// Shader program in use.
    ShaderProgram* shaderProgram_;

};

// System Memory��SM��������ϵͳ�ڴ棬�����System Memory�ϵ�Resource�ǲ���ֱ�ӱ�GPU���ʵģ�����GPU��˵�ǲ��ɼ��ģ�����Resource���뾭�������������õ�GPU����Ⱦ�С�
// AGP Memory��AM��: Ҳ��Share System Memory����Non - Local  Video Memory, ��ʵ����ϵͳ�ڴ��һ���֣�ֻ���ⲿ���ڴ��Ѿ���MappingΪ�Դ��һ���֣�����CPU���Է����ⲿ���ڴ棬����GPUҲ����ͨ��PCI - E / AGP����͸�������ⲿ���ڴ棬�����Դ�һ������GPU�����ٶȱ��Դ������Ͼ�����ӳ�䣬���һ�Ҫ��PCI - E���߽��ж�д��
// Video Memory��VM��: �Դ棬Ҳ����Local Video Memory, ֻ��GPU���Է��ʣ�CPU���ɼ�Ҳ���ܷ��ʣ��ٶ���졣

// D3DPOOL��
// D3DPOOL_DEFAULT��һ������VM��AM�С�������ϵͳ�ڴ���û�б��ݣ���Device Lostʱ��Ҫ�ͷź����´�������������D3DPOOL_DEFAULT�е������ǲ��ܱ�CPU LOCK�ģ������Ƕ�̬������������D3DPOOL_DEFAULT�е�VB IB RENDERTARGET BACK BUFFERS���Ա�LOCK��������D3DPOOL_DEFAULT������Դʱ������Դ��Ѿ�ʹ����ϣ����й���Դ�ᱻ�����Դ����ͷ��㹻�Ŀռ䡣��
// D3DPOOL_MANAGED����D3DPOOL_DEFAULT����������D3DRuntime����SM��Ϊ�䴴�����ݣ�Device Lost����Ҫ����D3DRuntime���Զ��ñ���ͨ���Կ��������´�������Դ����������ʱ�򱻷�����SM����GPU��Ҫʹ����ԴʱD3D RUNTIME�Զ������ݿ�����VM��ȥ������Դ��GPU�޸ĺ�RUNTIME�ڱ�Ҫʱ�Զ�������µ�SM����������SM���޸ĺ�Ҳ�ᱻUPDATE��VMȥ�С����Ա�CPU����GPUƵ���޸ĵ����ݣ�һ����Ҫʹ���й����ͣ�����������ǳ������ͬ����������
// D3DPOOL_SYSTEMMEM��ֻ�����SM�У�������Դ���ܱ�GPU���ʣ�Device Lost����Ҫ����
// D3DPOOL_SCRATCH��ֻ�����SM�У���ͬ��D3DPOOL_SYSTEMMEM����Щ��Դ����ͼ���豸����Լ�����ԣ��豸�޷����ʸ������ڴ���е���Դ������Щ��Դ֮����Ի��ิ�ơ�����D3DPOOL_SYSTEMMEM�����ʹ��D3DPOOL_SYSTEMMEMʱ����Դ��ʽ������Device���ܣ���Ϊ��Դ�ܿ��ܻᱻ���µ�AM��VM��ȥ��ͼ��ϵͳʹ�ã���SCRATCHֻ��RUNTIME���ƣ�����������Դ�޷���ͼ��ϵͳʹ�á� ��
// �ڴ���Resourceʱ����ָ�� D3DPOOLʱ��Щ���ƣ����紴��RenderTarget��DepthStencilֻ�ܱ�־ΪD3DPOOL_DEFAULT����ָ��Ϊdynamic��resourceֻ�ܱ�־ΪD3DPOOL_DEFAULT��D3DPOOL_SYSTEMMEM����ϸ�ɲο�D3D SDK �ĵ���
// �ܽ᣺���ڶ�̬��Դ��RenderTarget��DepthStencil��D3DUSAGE_DYNAMIC��Դ��������D3DPOOL_DEFAULT��Device Lost��������������
//       ���ھ�̬��Դ������Ҫ�����޸Ļ���ʵ����ݣ����磬���Ρ����н�������������D3DPOOL_DEFAULT��D3DPOOL_MANAGED��Device Lost�����й�ֱ�Ӵӱ����лָ�����

// D3DUSAGE��
// �Դ����D3DUSAGE_DYNAMIC:��������ʱ�����δʹ�ñ�� D3DUSAGE_DYNAMIC ������������Ļ���Ϊ��̬����(static buffer)����̬����һ�㱻�������Դ��У��Ա�֤�洢�����е����ݵõ����Ч�Ĵ���Ȼ������̬�������������Ծ�̬�����д�������ٶ�Ϊ���۵ģ�������ΪCPU�����Դ���ٶȱ���ͺ�������������ԭ�������þ�̬�������洢��̬����(��Щ����Ҫ�����޸Ļ���ʵ�����)����̬���������Ӧ�ó����ʼ��ʱ�ü���������ݽ�����䡣
// AGP��D3DUSAGE_DYNAMIC: ��������ʱ�����ʹ���˱�� D3DUSAGE_DYNAMIC �� ����������Ļ���Ϊ��̬����(dynamic buffer)����̬����һ������� AGP(Accelerated Graphics Port) �洢���У������ݿɱ�Ѹ�ٸ��¡���̬���������ݵĴ����ٶȲ���̬���������죬������Ϊ�ڻ���֮ǰ���ݱ��봫�䵽�Դ��С�����̬�����ͻ���ŵ���������ٶ��൱��(���ٵ� CPU д����)�����ԣ��������ҪƵ�����»����е����ݣ��û���Ӧ����Ϊ��̬�ġ�
// ���Դ��AGP�洢�����ж������ǳ��������ԣ������Ҫ�ڳ�������ʱ��ȡ�������ݣ������ϵͳ�ڴ��б���һ�ݸ�����Ȼ������Ҫʱ������ж�������

// LOCK
// 1�����LOCK DEFAULT��Դ�ᷢ��ʲô����أ�DEFAULT��Դ������VM��AM�У�
//      a�������VM�У�������ϵͳ�����п���һ����ʱ���巵�ظ����ݣ���Ӧ�ó���������䵽��ʱ�����UNLOCK��ʱ��RUNTIME�Ὣ��ʱ��������ݴ��ص�VM��ȥ�������ԴD3DUSAGE���Բ���WRITEONLY�ģ���ϵͳ����Ҫ�ȴ�VM����һ��ԭʼ���ݵ���ʱ�������������Ϊʲô��ָ��WRITEONLY�ή�ͳ������ܵ�ԭ��
//      b��CPUдAMҲ����Ҫע��ĵط�����ΪCPUдAMһ����WRITE COMBINING��Ҳ����˵��д���嵽һ��CACHE LINE�ϣ���CACHE LINE����֮���FLUSH��AM��ȥ����һ��Ҫע��ľ���д���ݱ�����WEAK ORDER�ģ�ͼ������һ�㶼�������Ҫ�󣩣��ڶ��뾡��һ��д��һ��CACHE LINE��������ж����ӳ٣���ΪCPUÿ�α���FLUSH����CACHE LINE��Ŀ�꣬���������ֻд��LINE�в����ֽڣ�CPU�����ȴ�AM�ж�ȡ����LINE������COMBINE������FLUSH������������˳��д�����д����WRITE COMBINING���������׸����������д��Դ����Ҫʹ��D3DUSAGE_DYNAMIC��������ʹ��D3DPOOL_MANAGED������д����ȫ��SM����ɡ�
// 2����ͨ����D3DPOOL_DEFAULT���ǲ��ܱ������ģ���Ϊ��λ��VM�У�ֻ��ͨ��UPDATESURFACE��UPDATETEXTURE��IDirect3DDevice9::UpdateSurface or IDirect3DDevice9::UpdateTexture�������ʣ�ΪʲôD3D��������������̬����ȴ������������̬VB IB�أ��Ҳ²������2�������ԭ��
//      a����һ�����������һ��ʮ���Ӵ���������GPU�ڲ��Ѷ�ά��ʽ�洢��
//      b���ڶ���������GPU�ڲ�����NATIVE FORMAT��ʽ�洢�ģ�����������enRGBA��ʽ����̬������Ϊ�������������Ҫ�����޸ģ�����D3D���ر�洢�Դ���
// 3����Ƶ���޸ĵĶ�̬�����ʺ��ö�̬���Դ������ڴ˷��������˵����һ����GPUд���RENDERTARGET��һ����CPUд���TEXTURE VIDEO������֪����̬��Դһ���Ƿ�����AM�еģ�GPU����AM��Ҫ����AGP / PCI - E���ߣ��ٶȽ�VM����࣬��CPU����AM�ֽ�SM���ܶ࣬�����ԴΪ��̬���ԣ���ζ��GPU��CPU������Դ��������ӳ٣����Դ�����Դ�����D3DPOOL_DEFAULT��D3DPOOL_SYSTEMMEM������һ�ݣ��Լ��ֶ�����˫����¸��á�
// 4��ǧ��� RENDERTARGET��D3DPOOL_MANAGED ���Դ���������Ч�ʼ��ͣ�ԭ���Լ������������ڸĶ���̫Ƶ������Դ���Ƽ�ʹ��DEFAULT�������Լ��ֶ����£���Ϊһ�θ��µ�Ч����ʧԶ��GPU��������AM��������ʧҪС��
// 5���������LOCK������Ӱ��������ܣ���Ϊһ��LOCK��Ҫ�ȴ�COMMAND BUFFERǰ��Ļ���ָ��ȫ��ִ����ϲ��ܷ��أ�����ܿ����޸�����ʹ�õ���Դ����LOCK���ص��޸����UNLOCK���ʱ��GPUȫ�����ڿ���״̬��û�к���ʹ��GPU��CPU�Ĳ����ԣ�DX8.0������һ���µ�LOCK��־D3DLOCK_DISCARD����ʾ�����ȡ��Դ��ֻ��ȫд��Դ������������RUNTIME������˸�����������������ظ�Ӧ�ó��������VM��ַָ�룬��ԭָ���ڱ���UNLOCK֮�󱻶�������ʹ�ã�����CPU LOCK����ȴ�GPUʹ����Դ��ϣ��ܼ�������ͼ����Դ�����㻺����������壩���⼼����VB IB������renaming����

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

// https://user.xmission.com/~legalize/book/preview/poster/pipeline-9.0.png
// ����D3DPOOL����ƪ���½��ķǳ���� https://blog.csdn.net/poem_of_sunshine/article/details/8619996

}
