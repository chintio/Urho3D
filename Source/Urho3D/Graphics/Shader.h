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

#include "../Container/ArrayPtr.h"
#include "../Resource/Resource.h"

namespace Urho3D
{

class ShaderVariation;

/// %Shader resource consisting of several shader variations.
class URHO3D_API Shader : public Resource
{
    URHO3D_OBJECT(Shader, Resource);

public:
    /// Construct.
    explicit Shader(Context* context);
    /// Destruct.
    ~Shader() override;
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;
    /// Finish resource loading. Always called from the main thread. Return true if successful.
    bool EndLoad() override;

    /// Return a variation with defines. Separate multiple defines with spaces.
    ShaderVariation* GetVariation(ShaderType type, const String& defines);
    /// Return a variation with defines. Separate multiple defines with spaces.
    ShaderVariation* GetVariation(ShaderType type, const char* defines);

    /// Return either vertex or pixel shader source code.
    const String& GetSourceCode(ShaderType type) const { return type == VS ? vsSourceCode_ : psSourceCode_; }

    /// Return the latest timestamp of the shader code and its includes.
    unsigned GetTimeStamp() const { return timeStamp_; } // ��ɫ��Դ�ļ�����������ļ���������ʱ���

private:
    /// Process source code and include files. Return true if successful.
    bool ProcessSource(String& code, Deserializer& source);
    /// Sort the defines and strip extra spaces to prevent creation of unnecessary duplicate shader variations.
    String NormalizeDefines(const String& defines);
    /// Recalculate the memory used by the shader.
    void RefreshMemoryUse();

    /// Source code adapted for vertex shader.
    String vsSourceCode_;
    /// Source code adapted for pixel shader.
    String psSourceCode_;
    /// Vertex shader variations.
    HashMap<StringHash, SharedPtr<ShaderVariation> > vsVariations_;
    /// Pixel shader variations.
    HashMap<StringHash, SharedPtr<ShaderVariation> > psVariations_;
    /// Source code timestamp.
    unsigned timeStamp_;
    /// Number of unique variations so far.
    unsigned numVariations_;
};

// Urho3Dʹ������ubershader�ķ�����ÿ����ɫ�������н�ʹ�ò�ͬ�ı���궨�幹���������ɾ�̬��static������Ƥ��skinned�����ӳ٣�deferred��������forward������Ӱ/����Ӱ��shadowed/unshadowed����Ⱦ��
// ��Щ���еĹ����ǰ�����еģ�technique��renderpath�����ļ�����������ɫ���Լ�������һ��ʹ�õı���궨�塣���⣬���潫����뼸�������ͺ�������ص����ö��塣ͨ��������Ԥ��ö�ٿ����ɵ�����ɫ�����������п��ܵ����С�
// ��Direct3D�ϣ��������ɫ���ֽ��뱣�浽���̵ġ�Cache����Ŀ¼�У�ÿ������궨���������һ����ɫ���ֽ����ļ������Ա��´���Ҫ��ɫ������ʱ�������ܺ�ʱ�ı��롣��OpenGL�ϣ����ֻ����ǲ����õġ�

// ���ñ���궨�壨compilation defines��
// ��Ⱦ��������ʱ������ϣ����Բ�ͬ�ļ��������ͺ��������������ض�����ɫ�����С���Щ��Ӧ�����±���궨�壺
// Vertex shader:
//     NUMVERTEXLIGHTS=1,2,3 or 4:Ӱ�����Ķ���ƹ���
//     DIRLIGHT, SPOTLIGHT, POINTLIGHT:ͨ������PERPIXEL��������ǰ����ս���ʹ��
//     SHADOW:������ǰ��������Ӱ
//     NORMALOFFSET:Ӧ���ݷ��ߵ�����Ӱ��������shadow receiver��UV����
//     SKINNED, INSTANCED, BILLBOARD:ѡ�񼸺�������
// Pixel shader:
//     DIRLIGHT, SPOTLIGHT, POINTLIGHT:ͨ������PERPIXEL��������ǰ����ս���ʹ��
//     CUBEMASK:���Դ������������ͼ����
//     SPEC:������ǰ�����о��淴�����
//     SHADOW:������ǰ��������Ӱ
//     SIMPLE_SHADOW, PCF_SHADOW, VSM_SHADOW:Ҫʹ�õ���Ӱ��������
//     SHADOWCMP:ʹ���ֶ���Ӱ��ȱȽϣ�Direct3D9��������DF16��DF24��Ӱ��ͼ��ʽ
//     HEIGHTFOG:����������и߶���ģʽ
// ������ɫ��ͳһֵ��shader uniforms��
// ������ÿ������λ�ã����淨�ߺ���������ȶ���Ҫ�ģ���ͳһֵ������Ϊ����ͼԪ�����򱣳ֲ������ɫ���������ݡ����ڶ�����ɫ����˵���������ձ��ͳһֵ���Ǳ任����
// һ��ͳһֵ��һ��ͼԪ�Ļ��ƹ������ǲ���ģ�������ֵ������glBegin��glEnd֮�����õ�
// uniform�޶��˱�ʾһ��������ֵ����Ӧ�ó�������ɫ��ִ��֮ǰָ����������ͼԪ�Ĵ�������в��ᷢ���仯��
// uniform�������ɶ�����ɫ����Ƭ����ɫ������ģ����Ǳ�������Ϊȫ�ֱ�����
// ����Ⱦ������ı���passesʱ�������ø�����������ͳһֵ��Э����Ⱦ����������ΪHLSL���������г����Ʒ��Ĳ����б��鿴Uniforms.glsl��ȡ��Ӧ��GLSL Uniforms��
// Vertex shader uniforms:
//     float3 cAmbientStartColor:���򻷾��⽥��Ŀ�ʼ��ɫֵ
//     float3 cAmbientEndColor:���򻷾��⽥��Ľ�����ɫֵ
//     float3 cCameraPos:�����������
//     float cNearClip:������ü�����
//     float cFarClip:���Զ�ü�����
//     float cDeltaTime:��ǰ֡��ʱ�䲽����֡��ʱ��
//     float4 cDepthMode:���ڼ���0-1֮����������ֵ�Ĳ������Դ��ݵ���ֵ���е�������ɫ����
//     float cElapsedTime:����������ʱ��ֵ��������ʵ�ֲ��ʶ���
//     float4x3 cModel:����Ⱦ���������任����
//     float4x3 cView:����Ĺ۲����
//     float4x3 cViewInv:����۲������棨�������任��
//     float4x4 cViewProj:����Ĺ۲����xͶӰ����
//     float4x3 cZone:����ı任�������ڻ����������
// Pixel shader uniforms:
//     float3 cAmbientColor:�޻������������Ļ�������ɫ
//     float3 cCameraPosPS:�����������
//     float4 cDepthReconstruct:�ӷ�����Ӳ��������������ؽ�0-1֮����������ֵ�Ĳ�����
//     float cDeltaTimePS:��ǰ֡��ʱ�䲽����֡��ʱ��
//     float cElapsedTimePS:����������ʱ��ֵ��
//     float3 cFogColor:���������ɫ
//     float4 cFogParams:�����������μ�Batch.cpp�Լ�Fog.hlsl��ȡȷ�к��壩
//     float cNearClipPS:������ü�����
//     float cFarClipPS:���Զ�ü�����
//     

// ��д��ɫ��
// ����ֱ�ΪHLSL��Direct3D����GLSL��OpenGL����д��ɫ����������ɫ��������������ɫ�������Ͼ�����ʵ����ͬ�Ĺ��ܡ�
// Ҫ��д�Լ�����ɫ�������Դ��������������ɫ����ʼ��Basic, Shadow & Unlit shaders����ɫ���������ⲿ�ļ�ͨ��ӵ��ͨ�õĹ��ܣ����磺Uniforms.hlsl, Samplers.hlsl & Transform.hlsl
// ת�����㣨������ʵ�ʵ���Ƥ��ʵ���򹫸��ƴ�����һ��ʹ�ú�ͺ�����ϵ�С���ɣ��ȫ�ķ��������ָ������´��룺
//     For HLSL:
//     float4x3 modelMatrix = iModelMatrix;
//     float3 worldPos = GetWorldPos(modelMatrix);
//     oPos = GetClipPos(worldPos);
//     For GLSL:
//     mat4 modelMatrix = iModelMatrix;
//     vec3 worldPos = GetWorldPos(modelMatrix);
//     gl_Position = GetClipPos(worldPos);
// ����Direct3D��OpenGL�������������ɫ����д��ͬһ���ļ���������ڵ㺯��������VS������PS��������OpenGLģʽ�£�����һ���ں�̨ת��ΪGLSL�����main�������������붥����ɫ��ʱ������궨�塰COMPILEVS��ʼ�մ��ڣ�����������ɫ��ʱҲ����֡�COMPILEPS������Щ����ɫ�������ļ��д���ʹ�ã��Է�ֹ�ԡ��������͵���ɫ����Ч�Ĺ��죬�����ٱ���ʱ�䡣
// ������ɫ��������Ҫ�붥��Ԫ������ƥ�������ȷ��Ⱦ������HLSL�У������������ÿ����ɫ�����ô�д���ʣ�POSITION��NORMAL��TEXCOORD0�ȣ����壬����GLSL�У�Ĭ�����Զ�����Transform.glsl��ͨ�������ִ�Сд���ַ�����contains�������붥��Ԫ������ƥ�䣬��ʹ�ÿ�ѡ�����ֺ�׺�������������������磬iTexCoord�ǵ�һ������������0���������꣬iTexCoord1�ǵڶ����������꣨��������1����
// ͳһֵ��Uniforms���������ض���ʽ��ǰ׺���Ա������ܹ�������ǣ�
//     c��ʾͳһ����������cMatDiffColor�����������ڲ�����ʱ��c�����룬���������SetShaderParameter�����г�Ϊ��MatDiffColor��
//     s��ʾ���������������sDiffMap��
// ��GLSL��ɫ���У�����������samplers��ָ������ȷ������Ԫ��texture units������Ҫ�����ʹ�õĲ��������Ʋ�����������Ԥ����ģ�����sDiffMap��ֻ��ȷ�������������е�ĳ��λ����һ�����֣�����������Ϊ����Ԫ�����磬������ɫ�������·�ʽʹ������Ԫ0-3��
//     uniform sampler2D sWeightMap0;
//     uniform sampler2D sDetailMap1;
//     uniform sampler2D sDetailMap2;
//     uniform sampler2D sDetailMap3;
// Ӳ����Ƥ��hardware skinning��֧�ֵ���������bones����ȡ����ͼ��API����ͨ���궨��MAXBONES���䴫�ݵ���ɫ�����롣ͨ�������ֵ��64������Raspberry PI�ϼ��ٵ�32����Direct3D 11��OpenGL 3�����ӵ�128����μ�GetMaxBones������

// API����
// Direct3D9��Direct3D11������ͬ��HLSL��ɫ�����룬ͬ����OpenGL 2��OpenGL 3��OpenGL ES 2��WebGL������ͬ��GLSL���롣���һЩ�������������ڿ��ܵ����������API���졣
// ΪDirect3D11����HLSL��ɫ��ʱ���궨��D3D11������֣���Ҫ����������ϸ��Ϣ��
//     ͳһֵ����֯�������������С��鿴�ļ�Uniforms.hlsl�������õ�ͳһֵ���鿴TerrainBlend.hlsl�е�ʾ���������Լ���ͳһֵ�����Զ��塱������������ۡ�
//     Ϊÿ������Ԫ��������Ͳ�������Samplers.hlsl��Sample2D��SampleCube�ȣ������ڱ�д��������api�Ϲ����Ĵ��롣���ǲ��ò�����s��ǰ׺������Ԫ���ơ�
//     ������ɫ�����λ�ú�������ɫ�������ɫ��Ҫʹ��SV_POSITION��SV_TARGET���塣��OUTPOSITION��OUTCOLOR0-3������������api��ѡ����ȷ�����塣�ڶ�����ɫ���У����λ��Ӧ���ָ�����������������������޷�����������ͨ����������ɫ���������������������ͬ��˳����Ϊ������ɫ�����롣����Direct3D��ɫ�����������ܻ�ָ����������塣
//     ��Direct3D11�ϣ������ֶ��������ƽ�����ꡣ����CLIPPLANE����궨���ʾ������Graphics���Զ���ӡ����磬��μ�LitSolid.hlsl��ɫ����
//     Direct3D11��֧�����ȣ�luminance��������alpha��luminance-alpha�������ʽ������ʹ��R��RGͨ������ˣ�׼���������ȡ��ִ���ʵ�����ת��
//     ���������ɫ�����ö��㻺�����в����ڵĶ���Ԫ�أ���Direct3D11���޷���Ⱦ��
// ����OpenGL����ΪOpenGL 3+����GLSL��ɫ��ʱ���궨��GL3������֣�����OpenGL ES 2���궨��GL_ES����֣�����WEBGL���궨��WEBGL������֣�����Raspberry Pi���궨��RPI������֡�ע�����²��죺
//     ��OpenGL 3�ϣ������ɫ��Դ����û�ж���汾����ʹ��GLSL�汾150���������������ͬ����ͨ��Samplers.glsl�еĺ궨����. ͬ�����ļ�Transform.glsl�������������������������ԡ���ֵ����Ƭ���������Ĳ��졣
//     ��OpenGL 3���ȣ�luminance���У�alpha��luminance alpha�����ʽ�����ã�����R��RG��ʽȡ������ˣ�׼���������ȡ��ִ���ʵ�����ת��
//     ��OpenGL ES 2����Ҫʹ�þ����޶�����

// ��ɫ��Ԥ�������棩
// ���ʼ����ڲ�ͬ��������������Ⱦ�����п���ʹ�õ���ɫ��������ڲ��ʼ���ʱö�٣��������������ܴ���������Ⱦ֮ǰ��ʵ���ϲ�����ֽ������������Щ��ɫ�����塣��������OpenGL�ϣ�����Ⱦ֮ǰ������ɫ�����ܻᵼ��֡���ʳ��ֹ��ϡ�Ϊ�˱���������������Խ�ʹ�ù�����ɫ�����ת����XML�ļ��У�Ȼ�����Ԥ���ء���μ�ͼ����ϵͳ�е�BeginDumpShaders������EndDumpShaders������PrecacheShaders�����������в��� - ds <file>������ָʾ����������ʱ��ʼ�Զ�ת����ɫ����
// ��ע�⣬ʹ�õ���ɫ�����彫��ͼ�����ö��仯������ ��Ӱ ���� ����/PCF/VSM��ʵ������/���á�

}
