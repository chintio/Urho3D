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

#include "../Math/Color.h"
#include "../Graphics/Drawable.h"
#include "../Math/Frustum.h"
#include "../Graphics/Texture.h"

namespace Urho3D
{

class Camera;
struct LightBatchQueue;

/// %Light types.
enum LightType
{
    LIGHT_DIRECTIONAL = 0,
    LIGHT_SPOT,
    LIGHT_POINT
};

static const float SHADOW_MIN_QUANTIZE = 0.1f;
static const float SHADOW_MIN_VIEW = 1.0f;
static const int MAX_LIGHT_SPLITS = 6;
#ifdef DESKTOP_GRAPHICS
static const unsigned MAX_CASCADE_SPLITS = 4;
#else
static const unsigned MAX_CASCADE_SPLITS = 1;
#endif

/// Depth bias parameters. Used both by lights (for shadow mapping) and materials.
struct URHO3D_API BiasParameters
{
    /// Construct undefined.
    BiasParameters() = default;

    /// Construct with initial values.
    BiasParameters(float constantBias, float slopeScaledBias, float normalOffset = 0.0f) :
        constantBias_(constantBias),
        slopeScaledBias_(slopeScaledBias),
        normalOffset_(normalOffset)
    {
    }

    /// Validate parameters.
    void Validate();

    /// Constant bias.
    float constantBias_;
    /// Slope scaled bias.
    float slopeScaledBias_;
    /// Normal offset multiplier.
    float normalOffset_;
};

/// Cascaded shadow map parameters.
struct URHO3D_API CascadeParameters
{
    /// Construct undefined.
    CascadeParameters() = default;

    /// Construct with initial values.
    CascadeParameters(float split1, float split2, float split3, float split4, float fadeStart, float biasAutoAdjust = 1.0f) :
        fadeStart_(fadeStart),
        biasAutoAdjust_(biasAutoAdjust)
    {
        splits_[0] = split1;
        splits_[1] = split2;
        splits_[2] = split3;
        splits_[3] = split4;
    }

    /// Validate parameters.
    void Validate();

    /// Return shadow maximum range.
    float GetShadowRange() const
    {
        float ret = 0.0f;
        for (unsigned i = 0; i < MAX_CASCADE_SPLITS; ++i)
            ret = Max(ret, splits_[i]);

        return ret;
    }

    /// Far clip values of the splits.
    Vector4 splits_;
    /// The point relative to the total shadow range where shadow fade begins (0.0 - 1.0)
    float fadeStart_{};
    /// Automatic depth bias adjustment strength.
    float biasAutoAdjust_{};
};

/// Shadow map focusing parameters.
struct URHO3D_API FocusParameters
{
    /// Construct undefined.
    FocusParameters() = default;

    /// Construct with initial values.
    FocusParameters(bool focus, bool nonUniform, bool autoSize, float quantize, float minView) :
        focus_(focus),
        nonUniform_(nonUniform),
        autoSize_(autoSize),
        quantize_(quantize),
        minView_(minView)
    {
    }

    /// Validate parameters.
    void Validate();

    /// Focus flag.
    bool focus_;
    /// Non-uniform focusing flag.
    bool nonUniform_;
    /// Auto-size (reduce resolution when far away) flag.
    bool autoSize_;
    /// Focus quantization.
    float quantize_;
    /// Minimum view size.
    float minView_;
};

// 照亮场景。可以选择投射阴影。
/// %Light component.
class URHO3D_API Light : public Drawable
{
    URHO3D_OBJECT(Light, Drawable);

public:
    /// Construct.
    explicit Light(Context* context);
    /// Destruct.
    ~Light() override;
    /// Register object factory. Drawable must be registered first.
    static void RegisterObject(Context* context);

    /// Process octree raycast. May be called from a worker thread.
    void ProcessRayQuery(const RayOctreeQuery& query, PODVector<RayQueryResult>& results) override;
    /// Calculate distance and prepare batches for rendering. May be called from worker thread(s), possibly re-entrantly.
    void UpdateBatches(const FrameInfo& frame) override;
    /// Visualize the component as debug geometry.
    void DrawDebugGeometry(DebugRenderer* debug, bool depthTest) override;

    /// Set light type.
    void SetLightType(LightType type);
    /// Set vertex lighting mode.
    void SetPerVertex(bool enable);
    /// Set color.
    void SetColor(const Color& color);
    /// Set temperature of the light in Kelvin. Modulates the light color when "use physical values" is enabled.
    void SetTemperature(float temperature);
    /// Set area light radius. Greater than zero activates area light mode. Works only with PBR shaders.
    void SetRadius(float radius);
    /// Set tube area light length. Works only with PBR shaders.
    void SetLength(float length);
    /// Set use physical light values.
    void SetUsePhysicalValues(bool enable);
    /// Set specular intensity. Zero disables specular calculations.
    void SetSpecularIntensity(float intensity);
    /// Set light brightness multiplier. Both the color and specular intensity are multiplied with this. When "use physical values" is enabled, the value is specified in lumens.
    void SetBrightness(float brightness);
    /// Set range.
    void SetRange(float range);
    /// Set spotlight field of view.
    void SetFov(float fov);
    /// Set spotlight aspect ratio.
    void SetAspectRatio(float aspectRatio);
    /// Set fade out start distance.
    void SetFadeDistance(float distance);
    /// Set shadow fade out start distance. Only has effect if shadow distance is also non-zero.
    void SetShadowFadeDistance(float distance);
    /// Set shadow depth bias parameters.
    void SetShadowBias(const BiasParameters& parameters);
    /// Set directional light cascaded shadow parameters.
    void SetShadowCascade(const CascadeParameters& parameters);
    /// Set shadow map focusing parameters.
    void SetShadowFocus(const FocusParameters& parameters);
    /// Set light intensity in shadow between 0.0 - 1.0. 0.0 (the default) gives fully dark shadows.
    void SetShadowIntensity(float intensity);
    /// Set shadow resolution between 0.25 - 1.0. Determines the shadow map to use.
    void SetShadowResolution(float resolution);
    /// Set shadow camera near/far clip distance ratio for spot and point lights. Does not affect directional lights, since they are orthographic and have near clip 0.
    void SetShadowNearFarRatio(float nearFarRatio);
    /// Set maximum shadow extrusion for directional lights. The actual extrusion will be the smaller of this and camera far clip. Default 1000.
    void SetShadowMaxExtrusion(float extrusion);
    /// Set range attenuation texture.
    void SetRampTexture(Texture* texture);
    /// Set spotlight attenuation texture.
    void SetShapeTexture(Texture* texture);

    /// Return light type.
    LightType GetLightType() const { return lightType_; }

    /// Return vertex lighting mode.
    bool GetPerVertex() const { return perVertex_; }

    /// Return color.
    const Color& GetColor() const { return color_; }

    /// Return the temperature of the light in Kelvin.
    float GetTemperature() const { return temperature_; }

    /// Return area light mode radius. Works only with PBR shaders.
    float GetRadius() const { return lightRad_; }

    /// Return area tube light length. Works only with PBR shaders.
    float GetLength() const { return lightLength_; }

    /// Return if light uses temperature and brightness in lumens.
    bool GetUsePhysicalValues() const { return usePhysicalValues_; }

    /// Return the color value of the temperature in Kelvin.
    Color GetColorFromTemperature() const;

    /// Return specular intensity.
    float GetSpecularIntensity() const { return specularIntensity_; }

    /// Return brightness multiplier. Specified in lumens when "use physical values" is enabled.
    float GetBrightness() const { return brightness_; }

    /// Return effective color, multiplied by brightness and affected by temperature when "use physical values" is enabled. Alpha is always 1 so that can compare against the default black color to detect a light with no effect.
    Color GetEffectiveColor() const;

    /// Return effective specular intensity, multiplied by absolute value of brightness.
    float GetEffectiveSpecularIntensity() const { return specularIntensity_ * Abs(brightness_); }

    /// Return range.
    float GetRange() const { return range_; }

    /// Return spotlight field of view.
    float GetFov() const { return fov_; }

    /// Return spotlight aspect ratio.
    float GetAspectRatio() const { return aspectRatio_; }

    /// Return fade start distance.
    float GetFadeDistance() const { return fadeDistance_; }

    /// Return shadow fade start distance.
    float GetShadowFadeDistance() const { return shadowFadeDistance_; }

    /// Return shadow depth bias parameters.
    const BiasParameters& GetShadowBias() const { return shadowBias_; }

    /// Return directional light cascaded shadow parameters.
    const CascadeParameters& GetShadowCascade() const { return shadowCascade_; }

    /// Return shadow map focus parameters.
    const FocusParameters& GetShadowFocus() const { return shadowFocus_; }

    /// Return light intensity in shadow.
    float GetShadowIntensity() const { return shadowIntensity_; }

    /// Return shadow resolution.
    float GetShadowResolution() const { return shadowResolution_; }

    /// Return shadow camera near/far clip distance ratio.
    float GetShadowNearFarRatio() const { return shadowNearFarRatio_; }

    /// Return maximum shadow extrusion distance for directional lights.
    float GetShadowMaxExtrusion() const { return shadowMaxExtrusion_; }

    /// Return range attenuation texture.
    Texture* GetRampTexture() const { return rampTexture_; }

    /// Return spotlight attenuation texture.
    Texture* GetShapeTexture() const { return shapeTexture_; }

    /// Return spotlight frustum.
    Frustum GetFrustum() const;
    /// Return spotlight frustum in the specified view space.
    Frustum GetViewSpaceFrustum(const Matrix3x4& view) const;

    /// Return number of shadow map cascade splits for a directional light, considering also graphics API limitations.
    int GetNumShadowSplits() const;

    /// Return whether light has negative (darkening) color.
    bool IsNegative() const { return GetEffectiveColor().SumRGB() < 0.0f; } // 返回灯光是否为负（变暗）颜色。

    /// Set sort value based on intensity and view distance.
    void SetIntensitySortValue(float distance);
    /// Set sort value based on overall intensity over a bounding box.
    void SetIntensitySortValue(const BoundingBox& box);
    /// Set light queue used for this light. Called by View.
    void SetLightQueue(LightBatchQueue* queue);
    /// Return light volume model transform.
    const Matrix3x4& GetVolumeTransform(Camera* camera);

    /// Return light queue. Called by View.
    LightBatchQueue* GetLightQueue() const { return lightQueue_; }

    /// Return a divisor value based on intensity for calculating the sort value.
    float GetIntensityDivisor(float attenuation = 1.0f) const
    {
        return Max(GetEffectiveColor().SumRGB(), 0.0f) * attenuation + M_EPSILON;
    }

    /// Set ramp texture attribute.
    void SetRampTextureAttr(const ResourceRef& value);
    /// Set shape texture attribute.
    void SetShapeTextureAttr(const ResourceRef& value);
    /// Return ramp texture attribute.
    ResourceRef GetRampTextureAttr() const;
    /// Return shape texture attribute.
    ResourceRef GetShapeTextureAttr() const;

    /// Return a transform for deferred fullscreen quad (directional light) rendering.
    static Matrix3x4 GetFullscreenQuadTransform(Camera* camera);

protected:
    /// Recalculate the world-space bounding box.
    void OnWorldBoundingBoxUpdate() override;

private:
    /// Validate shadow focus.
    void ValidateShadowFocus() { shadowFocus_.Validate(); }
    /// Validate shadow cascade.
    void ValidateShadowCascade() { shadowCascade_.Validate(); }
    /// Validate shadow bias.
    void ValidateShadowBias() { shadowBias_.Validate(); }
    /// Light type.
    LightType lightType_;
    /// Color.
    Color color_;
    /// Light temperature.
    float temperature_;
    /// Radius of the light source. If above 0 it will turn the light into an area light.  Works only with PBR shaders.
    float lightRad_;
    /// Length of the light source. If above 0 and radius is above 0 it will create a tube light. Works only with PBR shaders.
    float lightLength_;
    /// Shadow depth bias parameters.
    BiasParameters shadowBias_;
    /// Directional light cascaded shadow parameters.
    CascadeParameters shadowCascade_;
    /// Shadow map focus parameters.
    FocusParameters shadowFocus_;
    /// Custom world transform for the light volume.
    Matrix3x4 volumeTransform_;
    /// Range attenuation texture.
    SharedPtr<Texture> rampTexture_;
    /// Spotlight attenuation texture.
    SharedPtr<Texture> shapeTexture_;
    /// Light queue.
    LightBatchQueue* lightQueue_; // 该灯光的批次（像素光）
    /// Specular intensity.
    float specularIntensity_;
    /// Brightness multiplier.
    float brightness_;
    /// Range.
    float range_;
    /// Spotlight field of view.
    float fov_;
    /// Spotlight aspect ratio.
    float aspectRatio_;
    /// Fade start distance.
    float fadeDistance_;
    /// Shadow fade start distance.
    float shadowFadeDistance_;
    /// Light intensity in shadow.
    float shadowIntensity_;
    /// Shadow resolution.
    float shadowResolution_;
    /// Shadow camera near/far clip distance ratio.
    float shadowNearFarRatio_;
    /// Directional shadow max. extrusion distance.
    float shadowMaxExtrusion_; // 方向光阴影影响距离，物体距灯光超过这个距离，就不再计算它的阴影以节省场景运算
    /// Per-vertex lighting flag.
    bool perVertex_;
    /// Use physical light values flag.
    bool usePhysicalValues_;
};

// 顶点光排在前，像素光排在后（对于像素光sortValue_小的（最明亮/最接近相机）排在前）
inline bool CompareLights(Light* lhs, Light* rhs)
{
    // When sorting lights, give priority to per-vertex lights, so that vertex lit base pass can be evaluated first
    if (lhs->GetPerVertex() != rhs->GetPerVertex())
        return lhs->GetPerVertex();
    else
        return lhs->GetSortValue() < rhs->GetSortValue();
}

// Urho3D中的灯光可以是平行光（directional）、点光源（point）或聚光灯（spot），可以是逐像素（per-pixel）或逐顶点（per-vertex）。所有逐像素灯光都支持阴影映射。
// 平行光的位置没有影响，因为它被假定为无限远，只有它的旋转才重要。它投射正交投影的阴影。为了提高阴影质量，可以使用级联阴影贴图（cascaded shadow mapping）（沿Z轴将view拆分为多个阴影贴图）。
// 点光源是球形的。当一个点光源投射阴影时，它将在内部分割为6个聚光灯，每个聚光灯的FOV为90度。这是非常昂贵的渲染（rendering-wise），所以点光源投射阴影应谨慎使用。
// 聚光灯具有像相机那样的FOV和纵横比值，以定义光锥的形状。
// 逐像素模式的点光源和聚光灯都使用衰减渐变纹理（attenuation ramp texture）来确定强度如何随距离变化。此外，它们还有形状纹理（shape texture）、聚光灯的2D纹理和点光源的可选立方体纹理（cube texture）。聚光灯的形状纹理在边界处具有黑色，并且禁用mipmapping，这一点很重要，否则将在灯光圆锥体的边缘出现“出血”瑕疵。
// 通过调用SetPerVertex（）在灯光上启用逐顶点模式。逐顶点灯光在每个对象的环境光和雾计算期间进行计算，并且可以比每像素灯光快得多。当前每个对象最多有4个逐顶点灯光；如果超过此数目，将仅渲染影响对象的最亮逐顶点灯光。

// 灯光颜色（Light color）
// 灯光的颜色和强度由三个值控制：颜色（color）、镜面反射强度（specular intensity）和亮度倍增（brightness multiplier）。
// 亮度倍增（brightness multiplier）应用于颜色和镜面反射强度，以生成渲染中使用的最终值。这可以用来实现淡出或闪烁而不影响原始颜色。
// 镜面反射强度（specular intensity）为0将禁用逐像素灯光的镜面反射计算，从而加快GPU计算。逐顶点灯光从不使用高光计算。
// 通过将颜色分量（color components）或亮度倍增（brightness multiplier）设置为负值（negative value），可以实现负（减）光。这些可用于局部降低环境光照水平，例如创建一个黑暗的洞穴。负的逐像素灯光在灯光预过程（light pre-pass）渲染模式下不起作用，因为它使用具有黑色初始值的灯光累积缓冲区（light accumulation buffer），因此没有任何要减去的内容。
// 灯光还可以允许使用物理值，在这种情况下，亮度倍增以流明（lumens）为单位，而以开尔文（Kelvin）为单位的光温值也可用于调节颜色（通常情况下，颜色值本身将保留为白色）。请参见SetUsePhysicalValues（）和SetTemperature（）。

// 灯光剔除（Light culling）
// 使用遮挡时，如果灯光的包围盒（bounding box）完全位于遮挡器（occluder）后面，则灯光将自动被剔除。但是，平行光有一个无限的边界框，不能用这种方式剔除。
// 通过对灯光和对象调用SetLightMask（），可以限制受每个灯光影响的对象。对灯光和对象的光掩码（lightmasks）进行and运算，以检查灯光是否应具有效果：仅当结果为非零时，灯光才会照亮对象。默认情况下，对象和灯光在其光掩码（lightmasks）中设置了所有位，因此始终通过此测试。
// 区域（Zones）也可以用于灯光剔除。当一个对象在一个区域内时，它的光掩码将与该区域的光掩码进行AND运算，然后再对照灯光掩码对其进行测试。使用此机制，对象可以在场景中移动时动态更改其接受的灯光集。
// 使用灯光掩码进行灯光剔除时必须小心，因为它们很容易造成光线影响被非自然地切断的情况。然而，它们有助于防止光线溢出到不需要的区域，例如，一个房间内的灯光流入另一个房间，而不必求助于阴影投射（shadow-casting）灯光。
// 在灯光预处理（light pre-pass）和延迟渲染（deferred rendering）中，通过在G-buffer渲染期间将对象的光掩码写入模板缓冲区，并在渲染光体（light volumes）时将模板缓冲区与灯光的掩码进行比较来进行灯光剔除。在这种情况下，光掩码仅限于低8位。

// 阴影灯光（Shadowed lights）
// 阴影渲染很容易成为使用灯光最复杂的方面，因此存在大量用于控制阴影的每个灯光参数：
//     BiasParameters：定义常数和斜率比例的深度偏移值和法线偏移，以防止自阴影伪影（self-shadowing artifacts）。在实践中，需要通过实验来确定。正交（平行光）和投影（点光源和聚光灯）阴影可能需要非常不同的偏移值。法线偏移是另一种阴影偏移方法，它基于在接收器几何体法线方向上修改阴影接收器UV坐标，而不是在阴影渲染期间修改深度。另一种解决自阴影问题的方法是渲染阴影投射器（shadowcaster ）的背面，参见材料。
//     CascadeParameters：这些参数仅对平行光有效。它们指定每个级联阴影贴图分割（最大值4）的远裁剪距离（far clip distance），以及相对于最大阴影范围的淡入起始点。未使用的分割可以设置为“远剪裁0”。该结构还包括biasAutoAdjust设置，用于根据级联分割距离自动调整深度偏差。默认情况下，它在1x强度（值1）时处于启用状态，但可以禁用（值0）或调整得更强（值大于1）
//     FocusParameters：这些参数对平行光和聚光灯有影响，并且控制技术（techniques）提高阴影贴图的分辨率。它们包括焦点启用标志（允许在可见的阴影投射器和接收器上聚焦阴影摄影机）、非均匀缩放启用标志（允许更好的分辨率）、自动尺寸缩减标志（当光线远离时降低阴影贴图分辨率）以及阴影摄影机视图（view）的量化和最小尺寸参数。
// 此外，还有阴影淡入距离（shadow fade distance）、阴影强度（shadow intensity）、阴影分辨率（shadow resolution）、阴影近/远比（shadow near/far ratio）和阴影最大挤出（shadow max extrusion）参数：
//     如果“shadow distance”和“shadow fade distance”都大于零，则阴影在“shadow fade”距离处开始淡出，并在“shadow distance”处完全消失。
//     阴影强度定义阴影的暗度，介于0.0（最大暗度，默认值）和1.0（完全照亮）之间
//     “阴影分辨率”参数缩放渲染器中设置的全局阴影贴图大小，以确定实际的阴影贴图大小。最大值为1.0（全尺寸），最小值为0.125（八分之一大小）。请根据灯光的大小和重要性进行选择；较小的阴影贴图将不太需要性能。
//     “阴影近/远比”（shadow near/far ratio）控制点光源和聚光灯的“阴影摄影机近裁剪距离”（shadow camera near clip distance）。默认比率为0.002，这意味着范围为100的灯光的阴影摄影机在近平面的距离设置为0.2。将此值设置为尽可能高以获得更好的阴影深度分辨率，但请注意，可能还需要调整“偏移”参数。
//     “阴影最大挤出距离”（shadow max Extraction distance）控制平行光阴影摄影机的位置距离。有效值将是该参数和相机远裁剪距离的最小值。默认值为1000；如果您将阴影级联到很远的距离并使用高对象，请增加此值，并注意丢失阴影。挤出距离影响阴影贴图的深度分辨率，因此影响阴影偏移参数的效果。

// 全局阴影设置（Global shadow settings）
// 阴影贴图基础分辨率和质量（位深度和采样模式）是通过渲染器子系统中的函数设置的，请参见SetShadowMapSize（）和SetShadowQuality（）。
// 阴影质量枚举允许选择方差阴影（VSM（variance shadow map）），而不是默认的硬件深度阴影。VSM阴影的行为明显不同；深度偏移设置不再相关，但应确保所有大表面（包括地面和地形）都标记为阴影投射器（shadow casters），否则，由在其上移动的对象投射的阴影可能会显得不自然地稀疏。对于VSM阴影，请参见函数SetShadowSoftness（）和SetVSMShadowParameters（）来控制柔化（模糊）和阴影检测行为。与硬件深度阴影常见的自阴影瑕疵不同，当阴影投射表面彼此靠近灯光方向时，您可能会遇到光溢出（light bleeding），调整VSM shadow参数可能有帮助。
// VSM阴影贴图也可以进行多重采样以获得更好的质量，这有性能成本。请参见SetVSMMultiSample（）。

// 阴影贴图重用（Shadow map reuse）
// 渲染器（Renderer）可以配置为重用阴影贴图，也可以不重用。默认重用，请使用SetReuseShadowMaps（）进行更改。
// 启用“重用”后，各个阴影贴图尺寸只需要保留一个阴影纹理，并且在将单个阴影灯光的贡献渲染到不透明几何体之前，阴影贴图被渲染到"on the fly"。这有一个缺点，即在透明几何体渲染期间，阴影贴图不再可用，因此透明对象将不会接收阴影。
// 禁用“重用”时，将在实际场景渲染之前渲染所有阴影贴图。现在需要根据同时投射阴影的灯光数保留多个阴影纹理。请参见函数SetNumShadowMaps（）。如果没有足够的阴影纹理，它们将被指定给最近 / 最亮的灯光，其余的将被渲染为无阴影。现在需要更多的纹理内存，但优点是透明对象也可以接收阴影。

// 阴影剔除（Shadow culling）
// 与使用遮罩进行灯光剔除类似，阴影掩码可用于选择应针对每个灯光投射阴影的对象。请参见SetShadowMask（）。潜在的阴影投射器（shadow caster's）的阴影掩码将与灯光的掩码进行AND运算，以确定是否应将其渲染到灯光的阴影贴图中。另外，当一个物体在一个区域内时，它的阴影掩码也会和这个区域的阴影掩码一起被“与”在一起。默认情况下，所有位都设置在阴影掩码中。
// 作为阴影剔除的一个例子，想象一个房子（它本身就是一个阴影投射器），里面有几个物体，还有一个阴影平行光从窗户照进来。在这种情况下，可以通过从阴影掩码中清除相应的位来避免阴影贴图渲染。

}
