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
    unsigned GetTimeStamp() const { return timeStamp_; } // 着色器源文件（及其包含文件）的最新时间戳

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

// Urho3D使用类似ubershader的方法：每个着色器的排列将使用不同的编译宏定义构建，以生成静态（static）或蒙皮（skinned）、延迟（deferred）或正向（forward）或阴影/非阴影（shadowed/unshadowed）渲染。
// 这些排列的构建是按需进行的：technique和renderpath定义文件都引用了着色器以及与它们一起使用的编译宏定义。此外，引擎将添加与几何体类型和照明相关的内置定义。通常不可能预先枚举可以由单个着色器构建的所有可能的排列。
// 在Direct3D上，编译的着色器字节码保存到磁盘的“Cache”子目录中（每个编译宏定义组合生成一个着色器字节码文件），以便下次需要着色器排列时跳过可能耗时的编译。在OpenGL上，这种机制是不可用的。

// 内置编译宏定义
// 渲染场景对象时，引擎希望针对不同的几何体类型和照明条件存在特定的着色器排列。这些对应于以下编译宏定义：
// Vertex shader:
//     NUMVERTEXLIGHTS=1,2,3 or 4:影响对象的顶点灯光数
//     DIRLIGHT, SPOTLIGHT, POINTLIGHT:通过定义PERPIXEL，逐像素前向光照将被使用
//     SHADOW:逐像素前向光具有阴影
//     NORMALOFFSET:应根据法线调整阴影接收器（shadow receiver）UV坐标
//     SKINNED, INSTANCED, BILLBOARD:选择几何体类型
// Pixel shader:
//     DIRLIGHT, SPOTLIGHT, POINTLIGHT:通过定义PERPIXEL，逐像素前向光照将被使用
//     CUBEMASK:点光源具有立方体贴图掩码
//     SPEC:逐像素前向光具有镜面反射计算
//     SHADOW:逐像素前向光具有阴影
//     SIMPLE_SHADOW, PCF_SHADOW, VSM_SHADOW:要使用的阴影采样质量
//     SHADOWCMP:使用手动阴影深度比较，Direct3D9仅适用于DF16和DF24阴影贴图格式
//     HEIGHTFOG:对象区域具有高度雾模式

}
