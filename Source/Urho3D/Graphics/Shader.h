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

// 内置编译宏定义（compilation defines）
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
// 内置着色器统一值（shader uniforms）
// 属性是每个顶点位置，表面法线和纹理坐标等都需要的，而统一值则用于为整个图元批次向保持不变的着色器传递数据。对于顶点着色器来说，可能最普遍的统一值就是变换矩阵。
// 一个统一值在一个图元的绘制过程中是不变的，所以其值不能在glBegin和glEnd之间设置的
// uniform限定了表示一个变量的值将由应用程序在着色器执行之前指定，并且在图元的处理过程中不会发生变化。
// uniform变量是由顶点着色器和片段着色器共享的，他们必须声明为全局变量。
// 当渲染对象或四边形passes时，将设置各种引擎内置统一值以协助渲染。以下是作为HLSL数据类型列出的制服的部分列表。查看Uniforms.glsl获取相应的GLSL Uniforms。
// Vertex shader uniforms:
//     float3 cAmbientStartColor:区域环境光渐变的开始颜色值
//     float3 cAmbientEndColor:区域环境光渐变的结束颜色值
//     float3 cCameraPos:相机世界坐标
//     float cNearClip:相机近裁剪距离
//     float cFarClip:相机远裁剪距离
//     float cDeltaTime:当前帧的时间步长（帧耗时）
//     float4 cDepthMode:用于计算0-1之间的线性深度值的参数，以传递到插值器中的像素着色器。
//     float cElapsedTime:场景的运行时间值。可用于实现材质动画
//     float4x3 cModel:被渲染对象的世界变换矩阵
//     float4x3 cView:相机的观察矩阵
//     float4x3 cViewInv:相机观察矩阵的逆（相机世界变换）
//     float4x4 cViewProj:相机的观察矩阵x投影矩阵
//     float4x3 cZone:区域的变换矩阵；用于环境渐变计算
// Pixel shader uniforms:
//     float3 cAmbientColor:无环境渐变的区域的环境光颜色
//     float3 cCameraPosPS:相机世界坐标
//     float4 cDepthReconstruct:从非线性硬件深度纹理样本重建0-1之间的线性深度值的参数。
//     float cDeltaTimePS:当前帧的时间步长（帧耗时）
//     float cElapsedTimePS:场景的运行时间值。
//     float3 cFogColor:区域的雾颜色
//     float4 cFogParams:雾计算参数（参见Batch.cpp以及Fog.hlsl获取确切含义）
//     float cNearClipPS:相机近裁剪距离
//     float cFarClipPS:相机远裁剪距离
//     

// 编写着色器
// 必须分别为HLSL（Direct3D）和GLSL（OpenGL）编写着色器。内置着色器尝试在两种着色器语言上尽可能实现相同的功能。
// 要编写自己的着色器，可以从最基本的例子着色器开始：Basic, Shadow & Unlit shaders，着色器包含的外部文件通常拥有通用的功能，比如：Uniforms.hlsl, Samplers.hlsl & Transform.hlsl
// 转换顶点（隐藏了实际的蒙皮、实例或公告牌处理）是一种使用宏和函数组合的小技巧：最安全的方法是逐字复制以下代码：
//     For HLSL:
//     float4x3 modelMatrix = iModelMatrix;
//     float3 worldPos = GetWorldPos(modelMatrix);
//     oPos = GetClipPos(worldPos);
//     For GLSL:
//     mat4 modelMatrix = iModelMatrix;
//     vec3 worldPos = GetWorldPos(modelMatrix);
//     gl_Position = GetClipPos(worldPos);
// 对于Direct3D和OpenGL，顶点和像素着色器被写入同一个文件，并且入口点函数必须是VS（）和PS（）。在OpenGL模式下，其中一个在后台转换为GLSL所需的main（）函数。编译顶点着色器时，编译宏定义“COMPILEVS”始终存在，编译像素着色器时也会出现“COMPILEPS”。这些在着色器包含文件中大量使用，以防止对“错误”类型的着色器无效的构造，并减少编译时间。
// 顶点着色器输入需要与顶点元素语义匹配才能正确渲染。。在HLSL中，输入的语义在每个着色器中用大写单词（POSITION、NORMAL、TEXCOORD0等）定义，而在GLSL中，默认属性定义在Transform.glsl且通过不区分大小写的字符串“contains”操作与顶点元素语义匹配，并使用可选的数字后缀来定义语义索引。例如，iTexCoord是第一个（语义索引0）纹理坐标，iTexCoord1是第二个纹理坐标（语义索引1）。
// 统一值（Uniforms）必须以特定方式加前缀，以便引擎能够理解它们：
//     c表示统一常数，例如cMatDiffColor。当在引擎内部引用时，c被剥离，因此在例如SetShaderParameter（）中称为“MatDiffColor”
//     s表示纹理采样器，例如sDiffMap。
// 在GLSL着色器中，将采样器（samplers）指定给正确的纹理单元（texture units）很重要。如果使用的采样器名称不是在引擎中预定义的，例如sDiffMap，只需确保采样器名称中的某个位置有一个数字，它将被解释为纹理单元。例如，地形着色器按以下方式使用纹理单元0-3：
//     uniform sampler2D sWeightMap0;
//     uniform sampler2D sDetailMap1;
//     uniform sampler2D sDetailMap2;
//     uniform sampler2D sDetailMap3;
// 硬件蒙皮（hardware skinning）支持的最大骨骼（bones）数取决于图形API，并通过宏定义MAXBONES将其传递到着色器代码。通常，最大值是64，但在Raspberry PI上减少到32，在Direct3D 11和OpenGL 3上增加到128。请参见GetMaxBones（）。

// API差异
// Direct3D9和Direct3D11共享相同的HLSL着色器代码，同样，OpenGL 2、OpenGL 3、OpenGL ES 2和WebGL共享相同的GLSL代码。宏和一些条件代码用于在可能的情况下隐藏API差异。
// 为Direct3D11编译HLSL着色器时，宏定义D3D11将会出现，需要遵守以下详细信息：
//     统一值被组织到常量缓冲区中。查看文件Uniforms.hlsl关于内置的统一值。查看TerrainBlend.hlsl中的示例：定义自己的统一值到“自定义”常量缓冲区插槽。
//     为每个纹理单元定义纹理和采样器。Samplers.hlsl（Sample2D、SampleCube等）可用于编写在这两个api上工作的代码。它们采用不带“s”前缀的纹理单元名称。
//     顶点着色器输出位置和像素着色器输出颜色需要使用SV_POSITION和SV_TARGET语义。宏OUTPOSITION和OUTCOLOR0-3可用于在两个api上选择正确的语义。在顶点着色器中，输出位置应最后指定，否则其他输出语义可能无法正常工作。通常，顶点着色器定义的输出语义必须以相同的顺序定义为像素着色器输入。否则，Direct3D着色器编译器可能会指定错误的语义。
//     在Direct3D11上，必须手动计算剪裁平面坐标。这由CLIPPLANE编译宏定义表示，它由Graphics类自动添加。例如，请参见LitSolid.hlsl着色器。
//     Direct3D11不支持亮度（luminance）和亮度alpha（luminance-alpha）纹理格式，而是使用R和RG通道。因此，准备在纹理读取中执行适当的旋转。
//     如果顶点着色器引用顶点缓冲区中不存在的顶点元素，则Direct3D11将无法渲染。
// 对于OpenGL，当为OpenGL 3+编译GLSL着色器时，宏定义GL3将会出现；对于OpenGL ES 2，宏定义GL_ES会出现；对于WEBGL，宏定义WEBGL将会出现；对于Raspberry Pi，宏定义RPI将会出现。注意以下差异：
//     在OpenGL 3上，如果着色器源代码没有定义版本，则将使用GLSL版本150。纹理采样函数不同，但通过Samplers.glsl中的宏定义解决. 同样的文件Transform.glsl包含宏以隐藏在声明顶点属性、插值器和片段输出方面的差异。
//     在OpenGL 3亮度（luminance）中，alpha和luminance alpha纹理格式被弃用，并被R和RG格式取代。因此，准备在纹理读取中执行适当的旋转。
//     在OpenGL ES 2上需要使用精度限定符。

// 着色器预处理（缓存）
// 材质技术在不同的照明条件和渲染过程中可能使用的着色器变体会在材质加载时枚举，但由于其数量很大，在用于渲染之前，实际上不会从字节码编译或加载这些着色器变体。尤其是在OpenGL上，在渲染之前编译着色器可能会导致帧速率出现故障。为了避免这种情况，可以将使用过的着色器组合转储到XML文件中，然后进行预加载。请参见图形子系统中的BeginDumpShaders（）、EndDumpShaders（）和PrecacheShaders（）。命令行参数 - ds <file>可用于指示引擎在启动时开始自动转储着色器。
// 请注意，使用的着色器变体将随图形设置而变化，例如 阴影 质量 采样/PCF/VSM或实例启用/禁用。

}
