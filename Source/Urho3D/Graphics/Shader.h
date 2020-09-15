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

// ���ñ���궨��
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

}
