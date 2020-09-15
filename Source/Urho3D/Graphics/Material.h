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

#include "../Graphics/GraphicsDefs.h"
#include "../Graphics/Light.h"
#include "../Math/Vector4.h"
#include "../Resource/Resource.h"
#include "../Scene/ValueAnimationInfo.h"

namespace Urho3D
{

class Material;
class Pass;
class Scene;
class Technique;
class Texture;
class Texture2D;
class TextureCube;
class ValueAnimationInfo;
class JSONFile;

static const unsigned char DEFAULT_RENDER_ORDER = 128;

/// %Material's shader parameter definition.
struct MaterialShaderParameter
{
    /// Name.
    String name_;
    /// Value.
    Variant value_;
};

/// %Material's technique list entry.
struct TechniqueEntry
{
    /// Construct with defaults.
    TechniqueEntry() noexcept;
    /// Construct with parameters.
    TechniqueEntry(Technique* tech, MaterialQuality qualityLevel, float lodDistance) noexcept;
    /// Destruct.
    ~TechniqueEntry() noexcept = default;

    /// Technique.
    SharedPtr<Technique> technique_;
    /// Original technique, in case the material adds shader compilation defines. The modified clones are requested from it.
    SharedPtr<Technique> original_;
    /// Quality level.
    MaterialQuality qualityLevel_;
    /// LOD distance.
    float lodDistance_;
};

/// Material's shader parameter animation instance.
class ShaderParameterAnimationInfo : public ValueAnimationInfo
{
public:
    /// Construct.
    ShaderParameterAnimationInfo
        (Material* material, const String& name, ValueAnimation* attributeAnimation, WrapMode wrapMode, float speed);
    /// Copy construct.
    ShaderParameterAnimationInfo(const ShaderParameterAnimationInfo& other);
    /// Destruct.
    ~ShaderParameterAnimationInfo() override;

    /// Return shader parameter name.
    const String& GetName() const { return name_; }

protected:
    /// Apply new animation value to the target object. Called by Update().
    void ApplyValue(const Variant& newValue) override;

private:
    /// Shader parameter name.
    String name_;
};

/// TextureUnit hash function.
template <> inline unsigned MakeHash(const TextureUnit& value)
{
    return (unsigned)value;
}

/// Describes how to render 3D geometries.
class URHO3D_API Material : public Resource
{
    URHO3D_OBJECT(Material, Resource);

public:
    /// Construct.
    explicit Material(Context* context);
    /// Destruct.
    ~Material() override;
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;
    /// Finish resource loading. Always called from the main thread. Return true if successful.
    bool EndLoad() override;
    /// Save resource. Return true if successful.
    bool Save(Serializer& dest) const override;

    /// Load from an XML element. Return true if successful.
    bool Load(const XMLElement& source);
    /// Save to an XML element. Return true if successful.
    bool Save(XMLElement& dest) const;

    /// Load from a JSON value. Return true if successful.
    bool Load(const JSONValue& source);
    /// Save to a JSON value. Return true if successful.
    bool Save(JSONValue& dest) const;

    /// Set number of techniques.
    void SetNumTechniques(unsigned num);
    /// Set technique.
    void SetTechnique(unsigned index, Technique* tech, MaterialQuality qualityLevel = QUALITY_LOW, float lodDistance = 0.0f);
    /// Set additional vertex shader defines. Separate multiple defines with spaces. Setting defines at the material level causes technique(s) to be cloned as necessary.
    void SetVertexShaderDefines(const String& defines);
    /// Set additional pixel shader defines. Separate multiple defines with spaces. Setting defines at the material level causes technique(s) to be cloned as necessary.
    void SetPixelShaderDefines(const String& defines);
    /// Set shader parameter.
    void SetShaderParameter(const String& name, const Variant& value);
    /// Set shader parameter animation.
    void
        SetShaderParameterAnimation(const String& name, ValueAnimation* animation, WrapMode wrapMode = WM_LOOP, float speed = 1.0f);
    /// Set shader parameter animation wrap mode.
    void SetShaderParameterAnimationWrapMode(const String& name, WrapMode wrapMode);
    /// Set shader parameter animation speed.
    void SetShaderParameterAnimationSpeed(const String& name, float speed);
    /// Set texture.
    void SetTexture(TextureUnit unit, Texture* texture);
    /// Set texture coordinate transform.
    void SetUVTransform(const Vector2& offset, float rotation, const Vector2& repeat);
    /// Set texture coordinate transform.
    void SetUVTransform(const Vector2& offset, float rotation, float repeat);
    /// Set culling mode.
    void SetCullMode(CullMode mode);
    /// Set culling mode for shadows.
    void SetShadowCullMode(CullMode mode);
    /// Set polygon fill mode. Interacts with the camera's fill mode setting so that the "least filled" mode will be used.
    void SetFillMode(FillMode mode);
    /// Set depth bias parameters for depth write and compare. Note that the normal offset parameter is not used and will not be saved, as it affects only shadow map sampling during light rendering.
    void SetDepthBias(const BiasParameters& parameters);
    /// Set alpha-to-coverage mode on all passes.
    void SetAlphaToCoverage(bool enable);
    /// Set line antialiasing on/off. Has effect only on models that consist of line lists.
    void SetLineAntiAlias(bool enable);
    /// Set 8-bit render order within pass. Default 128. Lower values will render earlier and higher values later, taking precedence over e.g. state and distance sorting.
    void SetRenderOrder(unsigned char order);
    /// Set whether to use in occlusion rendering. Default true.
    void SetOcclusion(bool enable);
    /// Associate the material with a scene to ensure that shader parameter animation happens in sync with scene update, respecting the scene time scale. If no scene is set, the global update events will be used.
    void SetScene(Scene* scene);
    /// Remove shader parameter.
    void RemoveShaderParameter(const String& name);
    /// Reset all shader pointers.
    void ReleaseShaders();
    /// Clone the material.
    SharedPtr<Material> Clone(const String& cloneName = String::EMPTY) const;
    /// Ensure that material techniques are listed in correct order.
    void SortTechniques();
    /// Mark material for auxiliary view rendering.
    void MarkForAuxView(unsigned frameNumber);

    /// Return number of techniques.
    unsigned GetNumTechniques() const { return techniques_.Size(); }

    /// Return all techniques.
    const Vector<TechniqueEntry>& GetTechniques() const { return techniques_; }

    /// Return technique entry by index.
    const TechniqueEntry& GetTechniqueEntry(unsigned index) const;
    /// Return technique by index.
    Technique* GetTechnique(unsigned index) const;
    /// Return pass by technique index and pass name.
    Pass* GetPass(unsigned index, const String& passName) const;
    /// Return texture by unit.
    Texture* GetTexture(TextureUnit unit) const;

    /// Return all textures.
    const HashMap<TextureUnit, SharedPtr<Texture> >& GetTextures() const { return textures_; }

    /// Return additional vertex shader defines.
    const String& GetVertexShaderDefines() const { return vertexShaderDefines_; }
    /// Return additional pixel shader defines.
    const String& GetPixelShaderDefines() const { return pixelShaderDefines_; }

    /// Return shader parameter.
    const Variant& GetShaderParameter(const String& name) const;
    /// Return shader parameter animation.
    ValueAnimation* GetShaderParameterAnimation(const String& name) const;
    /// Return shader parameter animation wrap mode.
    WrapMode GetShaderParameterAnimationWrapMode(const String& name) const;
    /// Return shader parameter animation speed.
    float GetShaderParameterAnimationSpeed(const String& name) const;

    /// Return all shader parameters.
    const HashMap<StringHash, MaterialShaderParameter>& GetShaderParameters() const { return shaderParameters_; }

    /// Return normal culling mode.
    CullMode GetCullMode() const { return cullMode_; }

    /// Return culling mode for shadows.
    CullMode GetShadowCullMode() const { return shadowCullMode_; }

    /// Return polygon fill mode.
    FillMode GetFillMode() const { return fillMode_; }

    /// Return depth bias.
    const BiasParameters& GetDepthBias() const { return depthBias_; }

    /// Return alpha-to-coverage mode.
    bool GetAlphaToCoverage() const { return alphaToCoverage_; }

    /// Return whether line antialiasing is enabled.
    bool GetLineAntiAlias() const { return lineAntiAlias_; }

    /// Return render order.
    unsigned char GetRenderOrder() const { return renderOrder_; }

    /// Return last auxiliary view rendered frame number.
    unsigned GetAuxViewFrameNumber() const { return auxViewFrameNumber_; }

    /// Return whether should render occlusion.
    bool GetOcclusion() const { return occlusion_; }

    /// Return whether should render specular.
    bool GetSpecular() const { return specular_; }

    /// Return the scene associated with the material for shader parameter animation updates.
    Scene* GetScene() const;

    /// Return shader parameter hash value. Used as an optimization to avoid setting shader parameters unnecessarily.
    unsigned GetShaderParameterHash() const { return shaderParameterHash_; }

    /// Return name for texture unit.
    static String GetTextureUnitName(TextureUnit unit);
    /// Parse a shader parameter value from a string. Retunrs either a bool, a float, or a 2 to 4-component vector.
    static Variant ParseShaderParameterValue(const String& value);

private:
    /// Helper function for loading JSON files.
    bool BeginLoadJSON(Deserializer& source);
    /// Helper function for loading XML files.
    bool BeginLoadXML(Deserializer& source);

    /// Reset to defaults.
    void ResetToDefaults();
    /// Recalculate shader parameter hash.
    void RefreshShaderParameterHash();
    /// Recalculate the memory used by the material.
    void RefreshMemoryUse();
    /// Reapply shader defines to technique index. By default reapply all.
    void ApplyShaderDefines(unsigned index = M_MAX_UNSIGNED);
    /// Return shader parameter animation info.
    ShaderParameterAnimationInfo* GetShaderParameterAnimationInfo(const String& name) const;
    /// Update whether should be subscribed to scene or global update events for shader parameter animation.
    void UpdateEventSubscription();
    /// Update shader parameter animations.
    void HandleAttributeAnimationUpdate(StringHash eventType, VariantMap& eventData);

    /// Techniques.
    Vector<TechniqueEntry> techniques_;
    /// Textures.
    HashMap<TextureUnit, SharedPtr<Texture> > textures_;
    /// %Shader parameters.
    HashMap<StringHash, MaterialShaderParameter> shaderParameters_; // 材质对光照颜色的定义
    /// %Shader parameters animation infos.
    HashMap<StringHash, SharedPtr<ShaderParameterAnimationInfo> > shaderParameterAnimationInfos_; // 动画信息
    /// Vertex shader defines.
    String vertexShaderDefines_;
    /// Pixel shader defines.
    String pixelShaderDefines_;
    /// Normal culling mode.
    CullMode cullMode_{};
    /// Culling mode for shadow rendering.
    CullMode shadowCullMode_{};
    /// Polygon fill mode.
    FillMode fillMode_{};
    /// Depth bias parameters.
    BiasParameters depthBias_{};
    /// Render order value.
    unsigned char renderOrder_{};
    /// Last auxiliary view rendered frame number.
    unsigned auxViewFrameNumber_{};
    /// Shader parameter hash value.
    unsigned shaderParameterHash_{};
    /// Alpha-to-coverage flag.
    bool alphaToCoverage_{};
    /// Line antialiasing flag.
    bool lineAntiAlias_{};
    /// Render occlusion flag.
    bool occlusion_{true};
    /// Specular lighting flag.
    bool specular_{};
    /// Flag for whether is subscribed to animation updates.
    bool subscribed_{};
    /// Flag to suppress parameter hash and memory use recalculation when setting multiple shader parameters (loading or resetting the material.)
    bool batchedParameterUpdate_{};
    /// XML file used while loading.
    SharedPtr<XMLFile> loadXMLFile_;
    /// JSON file used while loading.
    SharedPtr<JSONFile> loadJSONFile_;
    /// Associated scene for shader parameter animation updates.
    WeakPtr<Scene> scene_;
};

// 材质（material）和技术（technique ）定义了如何渲染三维场景几何体：
// 材质（material）定义要使用的纹理（textures）、着色器参数（shader parameters）和消隐和填充（culling & fill）模式，并引用一种或多种技术（techniques）。
// 技术（technique ）定义实际渲染过程（passes）、每个渲染过程中要使用的着色器以及所有其他渲染状态，例如深度测试、深度写入和混合。

// 可以为不同的质量级别和LOD距离定义几种技术。技术质量等级从0（低）到2（高）。渲染时，将选择不超过渲染器（Renderer）的“材质质量”设置的最高可用技术（请参见SetMaterialQuality（））。
// 不同LOD级别和质量的技术必须按特定顺序出现：
//     距离最远，质量最高
//      。。。
//     距离最远，质量最低
//     距离第二远，质量最高
//      。。。

// 默认消隐模式是逆时针方向。shadowcull元素指定要在阴影过程（shadow pass）中使用的消隐模式。请注意，材质的“深度偏移”（depth bias）设置不适用于阴影过程；在阴影渲染过程中，将使用灯光的“深度偏移”（depth bias）。

// 渲染顺序（Render order）是一个8位无符号值，可用于影响过程（pass）、覆盖状态（overriding state）或距离排序（distance sorting）中的渲染顺序。默认值为128；较小的值将较早渲染，较大的值将在稍后渲染。“渲染顺序”（render order）的一个示例是确保在不透明物体渲染之后，再渲染使用“在像素中丢弃”（discard in pixel shader（ALPHAMASK define））的材质，以确保硬件深度缓冲区的行为最佳；在这种情况下，应增加渲染顺序。

// Occlusion flag允许禁用每个材质的软件遮挡渲染，例如，如果模型的某些部分是透明的。默认情况下启用遮挡。

// 材质可以选择设置“着色器编译宏定义”（vsdefines和psdefines）。在这种情况下，它们将被添加到技术（techniques）自己的编译宏定义中，并且根据需要克隆这些技术以确保唯一性。

// 在材质上“启用alpha覆盖（Enabling alpha-to-coverage ）可以在所有或每个过程（pass）中启用它。

// 漫反射贴图（Diffuse maps）指定RGB通道中的表面（surface ）颜色。他们可以选择使用alpha通道进行混合和alpha测试。它们最好压缩为DXT1（没有alpha或1位alpha）或DXT5（平滑alpha）格式。
// 法线贴图（Normal maps）编码切线空间（tangent-space）的表面法线（surface normal）。存储法线有两个选项，需要选择正确的材质技术（material technique），因为像素着色器在每种情况下都不同：
//     （1）存储为RGB。此时，使用DiffNormal技术。这是AssetImporter使用的默认值，以确保不需要发生法线纹理（normal textures）的转换。
//     （2）存储为xGxR，即Y分量在绿色通道中，X分量在alpha中。Z分量将在像素着色器中重建。这种编码适合DXT5压缩。您需要在材质中使用像素着色器宏定义PACKEDNORMAL；请参阅Stone示例材质。要将法线贴图转换为这种格式，可以使用AMD的compressionator实用程序
// 确保法线贴图的方向正确：平坦表面（even surface）的颜色值应为R 0.5， G 0.5， B 1.0
// 使用法线贴图材质的模型需要在其顶点数据中包含切线向量（tangent vectors）；确保这一点的最简单方法是使用AssetImporter或OgreImporter将模型导入到Urho3D格式时使用switch -t（生成切线）。如果没有切线，法线贴图材质上的灯光衰减将以完全不稳定的方式表现。
// 高光贴图（Specular maps）将高光表面颜色编码为RGB。请注意，延迟渲染只能使用来自G通道的单色镜面反射（specular）强度，而前向（forward）和灯光预处理（light pre-pass）渲染使用全彩色镜面反射。DXT1格式应该很适合这些纹理。
// 纹理（Textures）可以有一个附带的XML文件，该文件指定加载时（load-time）参数，例如寻址、mipmapping和每个质量级别上要跳过的mip等级的数量：
//     sRGB标志控制：1，是否应使用sRGB色彩到线性色彩的转换对纹理进行采样，如果用作rendertarget，则像素在写入时应转换回sRGB。2，控制backbuffer是否应在写入时使用sRGB转换，请在图形（Graphics）子系统上调用SetSRGB()。
//     可以选择指定各向异性级别（Anisotropy level）。如果省略（或者如果指定值0），将使用Renderer类中的默认值。

}
