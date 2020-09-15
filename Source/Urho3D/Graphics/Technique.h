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

/// \file

#pragma once

#include "../Graphics/GraphicsDefs.h"
#include "../Resource/Resource.h"

namespace Urho3D
{

class ShaderVariation;

/// Lighting mode of a pass.
enum PassLightingMode
{
    LIGHTING_UNLIT = 0,
    LIGHTING_PERVERTEX,
    LIGHTING_PERPIXEL
};

/// %Material rendering pass, which defines shaders and render state.
class URHO3D_API Pass : public RefCounted
{
public:
    /// Construct.
    explicit Pass(const String& name);
    /// Destruct.
    ~Pass() override;

    /// Set blend mode.
    void SetBlendMode(BlendMode mode);
    /// Set culling mode override. By default culling mode is read from the material instead. Set the illegal culling mode MAX_CULLMODES to disable override again.
    void SetCullMode(CullMode mode);
    /// Set depth compare mode.
    void SetDepthTestMode(CompareMode mode);
    /// Set pass lighting mode, affects what shader variations will be attempted to be loaded.
    void SetLightingMode(PassLightingMode mode);
    /// Set depth write on/off.
    void SetDepthWrite(bool enable);
    /// Set alpha-to-coverage on/off.
    void SetAlphaToCoverage(bool enable);
    /// Set whether requires desktop level hardware.
    void SetIsDesktop(bool enable);
    /// Set vertex shader name.
    void SetVertexShader(const String& name);
    /// Set pixel shader name.
    void SetPixelShader(const String& name);
    /// Set vertex shader defines. Separate multiple defines with spaces.
    void SetVertexShaderDefines(const String& defines);
    /// Set pixel shader defines. Separate multiple defines with spaces.
    void SetPixelShaderDefines(const String& defines);
    /// Set vertex shader define excludes. Use to mark defines that the shader code will not recognize, to prevent compiling redundant shader variations.
    void SetVertexShaderDefineExcludes(const String& excludes);
    /// Set pixel shader define excludes. Use to mark defines that the shader code will not recognize, to prevent compiling redundant shader variations.
    void SetPixelShaderDefineExcludes(const String& excludes);
    /// Reset shader pointers.
    void ReleaseShaders();
    /// Mark shaders loaded this frame.
    void MarkShadersLoaded(unsigned frameNumber);

    /// Return pass name.
    const String& GetName() const { return name_; }

    /// Return pass index. This is used for optimal render-time pass queries that avoid map lookups.
    unsigned GetIndex() const { return index_; }

    /// Return blend mode.
    BlendMode GetBlendMode() const { return blendMode_; }

    /// Return culling mode override. If pass is not overriding culling mode (default), the illegal mode MAX_CULLMODES is returned.
    CullMode GetCullMode() const { return cullMode_; }

    /// Return depth compare mode.
    CompareMode GetDepthTestMode() const { return depthTestMode_; }

    /// Return pass lighting mode.
    PassLightingMode GetLightingMode() const { return lightingMode_; }

    /// Return last shaders loaded frame number.
    unsigned GetShadersLoadedFrameNumber() const { return shadersLoadedFrameNumber_; }

    /// Return depth write mode.
    bool GetDepthWrite() const { return depthWrite_; }

    /// Return alpha-to-coverage mode.
    bool GetAlphaToCoverage() const { return alphaToCoverage_; }

    /// Return whether requires desktop level hardware.
    bool IsDesktop() const { return isDesktop_; }

    /// Return vertex shader name.
    const String& GetVertexShader() const { return vertexShaderName_; }

    /// Return pixel shader name.
    const String& GetPixelShader() const { return pixelShaderName_; }

    /// Return vertex shader defines.
    const String& GetVertexShaderDefines() const { return vertexShaderDefines_; }

    /// Return pixel shader defines.
    const String& GetPixelShaderDefines() const { return pixelShaderDefines_; }

    /// Return vertex shader define excludes.
    const String& GetVertexShaderDefineExcludes() const { return vertexShaderDefineExcludes_; }

    /// Return pixel shader define excludes.
    const String& GetPixelShaderDefineExcludes() const { return pixelShaderDefineExcludes_; }

    /// Return vertex shaders.
    Vector<SharedPtr<ShaderVariation> >& GetVertexShaders() { return vertexShaders_; }

    /// Return pixel shaders.
    Vector<SharedPtr<ShaderVariation> >& GetPixelShaders() { return pixelShaders_; }

    /// Return vertex shaders with extra defines from the renderpath.
    Vector<SharedPtr<ShaderVariation> >& GetVertexShaders(const StringHash& extraDefinesHash);
    /// Return pixel shaders with extra defines from the renderpath.
    Vector<SharedPtr<ShaderVariation> >& GetPixelShaders(const StringHash& extraDefinesHash);
    /// Return the effective vertex shader defines, accounting for excludes. Called internally by Renderer.
    String GetEffectiveVertexShaderDefines() const;
    /// Return the effective pixel shader defines, accounting for excludes. Called internally by Renderer.
    String GetEffectivePixelShaderDefines() const;

private:
    /// Pass index.
    unsigned index_;
    /// Blend mode.
    BlendMode blendMode_;
    /// Culling mode.
    CullMode cullMode_;
    /// Depth compare mode.
    CompareMode depthTestMode_;
    /// Lighting mode.
    PassLightingMode lightingMode_;
    /// Last shaders loaded frame number.
    unsigned shadersLoadedFrameNumber_;
    /// Depth write mode.
    bool depthWrite_;
    /// Alpha-to-coverage mode.
    bool alphaToCoverage_;
    /// Require desktop level hardware flag.
    bool isDesktop_;
    /// Vertex shader name.
    String vertexShaderName_;
    /// Pixel shader name.
    String pixelShaderName_;
    /// Vertex shader defines.
    String vertexShaderDefines_; // 着色器宏定义，如果pass中有"vs"属性，则使用pass中的"vsdefines"，否则使用technique中的"vsdefines"+pass中的"vsdefines"
    /// Pixel shader defines.
    String pixelShaderDefines_;
    /// Vertex shader define excludes.
    String vertexShaderDefineExcludes_; // 需要排除的着色器宏定义
    /// Pixel shader define excludes.
    String pixelShaderDefineExcludes_;
    /// Vertex shaders.
    Vector<SharedPtr<ShaderVariation> > vertexShaders_;
    /// Pixel shaders.
    Vector<SharedPtr<ShaderVariation> > pixelShaders_;
    /// Vertex shaders with extra defines from the renderpath.
    HashMap<StringHash, Vector<SharedPtr<ShaderVariation> > > extraVertexShaders_;
    /// Pixel shaders with extra defines from the renderpath.
    HashMap<StringHash, Vector<SharedPtr<ShaderVariation> > > extraPixelShaders_;
    /// Pass name.
    String name_;
};

/// %Material technique. Consists of several passes.
class URHO3D_API Technique : public Resource
{
    URHO3D_OBJECT(Technique, Resource);

    friend class Renderer;

public:
    /// Construct.
    explicit Technique(Context* context);
    /// Destruct.
    ~Technique() override;
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;

    /// Set whether requires desktop level hardware.
    void SetIsDesktop(bool enable);
    /// Create a new pass.
    Pass* CreatePass(const String& name);
    /// Remove a pass.
    void RemovePass(const String& name);
    /// Reset shader pointers in all passes.
    void ReleaseShaders();
    /// Clone the technique. Passes will be deep copied to allow independent modification.
    SharedPtr<Technique> Clone(const String& cloneName = String::EMPTY) const;

    /// Return whether requires desktop level hardware.
    bool IsDesktop() const { return isDesktop_; }

    /// Return whether technique is supported by the current hardware.
    bool IsSupported() const { return !isDesktop_ || desktopSupport_; }

    /// Return whether has a pass.
    bool HasPass(unsigned passIndex) const { return passIndex < passes_.Size() && passes_[passIndex].Get() != nullptr; }

    /// Return whether has a pass by name. This overload should not be called in time-critical rendering loops; use a pre-acquired pass index instead.
    bool HasPass(const String& name) const;

    /// Return a pass, or null if not found.
    Pass* GetPass(unsigned passIndex) const { return passIndex < passes_.Size() ? passes_[passIndex].Get() : nullptr; }

    /// Return a pass by name, or null if not found. This overload should not be called in time-critical rendering loops; use a pre-acquired pass index instead.
    Pass* GetPass(const String& name) const;

    /// Return a pass that is supported for rendering, or null if not found.
    Pass* GetSupportedPass(unsigned passIndex) const
    {
        Pass* pass = passIndex < passes_.Size() ? passes_[passIndex].Get() : nullptr;
        return pass && (!pass->IsDesktop() || desktopSupport_) ? pass : nullptr;
    }

    /// Return a supported pass by name. This overload should not be called in time-critical rendering loops; use a pre-acquired pass index instead.
    Pass* GetSupportedPass(const String& name) const;

    /// Return number of passes.
    unsigned GetNumPasses() const;
    /// Return all pass names.
    Vector<String> GetPassNames() const;
    /// Return all passes.
    PODVector<Pass*> GetPasses() const;

    /// Return a clone with added shader compilation defines. Called internally by Material.
    SharedPtr<Technique> CloneWithDefines(const String& vsDefines, const String& psDefines);

    /// Return a pass type index by name. Allocate new if not used yet.
    static unsigned GetPassIndex(const String& passName);

    /// Index for base pass. Initialized once GetPassIndex() has been called for the first time.
    static unsigned basePassIndex;
    /// Index for alpha pass. Initialized once GetPassIndex() has been called for the first time.
    static unsigned alphaPassIndex;
    /// Index for prepass material pass. Initialized once GetPassIndex() has been called for the first time.
    static unsigned materialPassIndex;
    /// Index for deferred G-buffer pass. Initialized once GetPassIndex() has been called for the first time.
    static unsigned deferredPassIndex;
    /// Index for per-pixel light pass. Initialized once GetPassIndex() has been called for the first time.
    static unsigned lightPassIndex;
    /// Index for lit base pass. Initialized once GetPassIndex() has been called for the first time.
    static unsigned litBasePassIndex;
    /// Index for lit alpha pass. Initialized once GetPassIndex() has been called for the first time.
    static unsigned litAlphaPassIndex;
    /// Index for shadow pass. Initialized once GetPassIndex() has been called for the first time.
    static unsigned shadowPassIndex;

private:
    /// Require desktop GPU flag.
    bool isDesktop_;
    /// Cached desktop GPU support flag.
    bool desktopSupport_;
    /// Passes.
    Vector<SharedPtr<Pass> > passes_;
    /// Cached clones with added shader compilation defines.
    HashMap<Pair<StringHash, StringHash>, SharedPtr<Technique> > cloneTechniques_;

    /// Pass index assignments.
    static HashMap<String, unsigned> passIndices;
};

// technology或pass中的“desktop”属性允许指定它需要桌面图形硬件（不包括移动设备）。
// pass通常不应定义剔除模式，但可以选择指定该模式以覆盖材质中的值。
// 通过指定不带路径和文件扩展名的着色器名称来引用着色器。例如“Basic”或“LitSolid”。引擎将添加正确的路径和文件扩展名（Direct3D的Shaders/HLSL/LitSolid.hlsl，OpenGL的Shaders/GLSL/LitSolid.glsl）自动执行。同一着色器源文件同时包含顶点着色器和像素着色器。此外，可以指定编译宏定义，并将其传递给着色器编译器。例如，“DIFFMAP”通常在像素着色器中启用漫反射贴图。
// 着色器及其编译宏定义可以在technique和pass级别上指定。如果pass没有覆盖在technique级别上指定的默认着色器，它仍然可以指定要使用的其他编译宏定义。但是，如果pass覆盖着色器，则不使用technique级别的宏定义。
// 由于材质可以设置进一步的着色器定义（将应用于所有pass），“vsexcludes”和“psexcludes”机制允许对每一个pass进行控制，以防止被包含。这是为了消除不必要的着色器变异编译，例如阴影着色器尝试读取法线贴图。
// technique不需要枚举用于不同几何体类型（非蒙皮、蒙皮、实例化、公告牌）以及不同的逐顶点和逐像素光组合的着色器。相反，引擎将为它们添加某些硬编码编译宏定义。

/* 不同pass的目的：
- base: 渲染不透明对象的环境光、逐顶点光和雾。
- litbase: 渲染不透明对象的第一个逐像素光、环境光和雾。这是优化的可选过程。
- light: 为不透明对象附加渲染一个逐像素光的贡献。
- alpha: 为透明对象渲染环境光、逐顶点光和雾。
- litalpha: 为透明对象附加渲染一个逐像素光的贡献。
- postopaque: 在不透明几何体之后的自定义渲染过程。可用于渲染skybox。
- refract: 在postopaque后的自定义渲染过程。可以从“环境纹理”（environment texture）单元采样“视口纹理”（viewport texture）以渲染折射对象。
- postalpha: 透明几何体之后的自定义渲染过程。
- prepass: 仅限灯光预处理（Light pre-pass）-将法线（normals）、高光（specular power）和深度（depth）渲染到G-buffer。
- material: 仅限灯光预处理（Light pre-pass）-通过组合环境光（ambient light）、逐顶点光（per-vertex lights）和逐像素光（per-pixel light）累积来渲染不透明几何体最终颜色。
- deferred: 仅限延迟渲染（Deferred rendering）-将环境光和逐顶点光渲染到rendertarget，并将漫反射反照率（diffuse albedo）、法线（normals）、镜面反射强度+功率（specular intensity + power）和深度（depth ）渲染到G-buffer。
- depth: 将线性深度渲染到rendertarget以获得后期处理效果。
- shadow: 渲染到硬件阴影贴图（仅深度）以生成阴影贴图。
*/
// 可以在渲染路径（render path）定义中定义和引用更多自定义过程（pass）。对于上面列出的内置过程，将自动识别要加载的光照着色器排列（shader permutations）（未照明（unlit）、逐顶点（per-vertex）或逐像素（per-pixel）），但对于自定义过程，需要显式指定这些排列（permutations）。默认值为unlit。
// 可选的“litbase”过程通过将环境光与影响对象的第一个逐像素灯光相结合来减少绘制调用（draw call）计数。但是，它有限制，不需要太多的着色器排列（shader permutations）：必须没有影响对象的顶点灯光，并且环境光不能有渐变。在过度透支的情况下，最好不要定义它，而是允许base pass（在计算上非常轻量级）先运行，为以后的过程初始化Z缓冲区。
// 折射过程（refract pass）需要将场景rendertarget乒乓化（pingponging）到纹理，但如果没有要渲染的折射几何体，则不会执行此操作，因此不会产生不必要的开销。

// 渲染顺序（Render order）注意事项：
// 当知道材质将仅渲染单个pass（例如延迟的G缓冲区过程）时，“渲染顺序”（Render order）效果良好。但是，当使用正向渲染（forward rendering）和每像素灯光（per-pixel lights）时，典型的照明几何体的渲染可以在“base”、“litbase”和“light”过程中分割。如果将“渲染顺序”与“深度测试”操作结合使用，以强制某些对象在其他对象之前进行渲染，则过程（pass）顺序可能不明显。“Base”过程将首先渲染，但对象通常不使用它，而是在以后使用“litbase”过程作为前向灯光循环的一部分进行渲染。这可能会导致深度测试操作失败，并导致与预期不同的结果。
// 一个简单的修复方法是完全禁用“litbase”优化过程，尽管这会降低性能。这可以从forward renderpath全局执行（Bin/CoreData/RenderPaths/Forward.xml)通过将forwardlights命令修改为：<command type="forwardlights" pass="light" uselitbase="false" />

}
