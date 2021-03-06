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

#include "../Container/Ptr.h"
#include "../Graphics/Drawable.h"
#include "../Graphics/Material.h"
#include "../Math/MathDefs.h"
#include "../Math/Matrix3x4.h"
#include "../Math/Rect.h"

namespace Urho3D
{

class Camera;
class Drawable;
class Geometry;
class Light;
class Material;
class Matrix3x4;
class Pass;
class ShaderVariation;
class Texture2D;
class VertexBuffer;
class View;
class Zone;
struct LightBatchQueue;

/// Queued 3D geometry draw call.
struct Batch
{
    /// Construct with defaults.
    Batch() = default;

    /// Construct from a drawable's source batch.
    explicit Batch(const SourceBatch& rhs) :
        distance_(rhs.distance_),
        renderOrder_(rhs.material_ ? rhs.material_->GetRenderOrder() : DEFAULT_RENDER_ORDER),
        isBase_(false),
        geometry_(rhs.geometry_),
        material_(rhs.material_),
        worldTransform_(rhs.worldTransform_),
        numWorldTransforms_(rhs.numWorldTransforms_),
        instancingData_(rhs.instancingData_),
        lightQueue_(nullptr),
        geometryType_(rhs.geometryType_)
    {
    }

    /// Calculate state sorting key, which consists of base pass flag, light, pass and geometry.
    void CalculateSortKey();
    /// Prepare for rendering.
    void Prepare(View* view, Camera* camera, bool setModelTransform, bool allowDepthWrite) const;
    /// Prepare and draw.
    void Draw(View* view, Camera* camera, bool allowDepthWrite) const;

    /// State sorting key.
    unsigned long long sortKey_{};
    /// Distance from camera.
    float distance_{}; // 与相机的距离
    /// 8-bit render order modifier from material.
    unsigned char renderOrder_{}; // material中定义的渲染顺序
    /// 8-bit light mask for stencil marking in deferred rendering.
    unsigned char lightMask_{};
    /// Base batch flag. This tells to draw the object fully without light optimizations.
    bool isBase_{};
    /// Geometry.
    Geometry* geometry_{};
    /// Material.
    Material* material_{};
    /// World transform(s). For a skinned model, these are the bone transforms.
    const Matrix3x4* worldTransform_{};
    /// Number of world transforms.
    unsigned numWorldTransforms_{};
    /// Per-instance data. If not null, must contain enough data to fill instancing buffer.
    void* instancingData_{};
    /// Zone.
    Zone* zone_{}; // drawable所在区域
    /// Light properties.
    LightBatchQueue* lightQueue_{}; // 批次所在的光源信息（像素光、顶点光、或空）
    /// Material pass.
    Pass* pass_{}; // 根据drawable->lodDistance_从material中选择最优technique，根据RenderPath中的scene pass从technique选择相应的pass
    /// Vertex shader.
    ShaderVariation* vertexShader_{};
    /// Pixel shader.
    ShaderVariation* pixelShader_{};
    /// %Geometry type.
    GeometryType geometryType_{};
};

/// Data for one geometry instance.
struct InstanceData
{
    /// Construct undefined.
    InstanceData() = default;

    /// Construct with transform, instancing data and distance.
    InstanceData(const Matrix3x4* worldTransform, const void* instancingData, float distance) :
        worldTransform_(worldTransform),
        instancingData_(instancingData),
        distance_(distance)
    {
    }

    /// World transform.
    const Matrix3x4* worldTransform_{};
    /// Instancing data buffer.
    const void* instancingData_{};
    /// Distance from camera.
    float distance_{};
};

/// Instanced 3D geometry draw call.
struct BatchGroup : public Batch
{
    /// Construct with defaults.
    BatchGroup() :
        startIndex_(M_MAX_UNSIGNED)
    {
    }

    /// Construct from a batch.
    explicit BatchGroup(const Batch& batch) :
        Batch(batch),
        startIndex_(M_MAX_UNSIGNED)
    {
    }

    /// Destruct.
    ~BatchGroup() = default;

    /// Add world transform(s) from a batch.
    void AddTransforms(const Batch& batch)
    {
        InstanceData newInstance;
        newInstance.distance_ = batch.distance_;
        newInstance.instancingData_ = batch.instancingData_;

        for (unsigned i = 0; i < batch.numWorldTransforms_; ++i)
        {
            newInstance.worldTransform_ = &batch.worldTransform_[i];
            instances_.Push(newInstance);
        }
    }

    /// Pre-set the instance data. Buffer must be big enough to hold all data.
    void SetInstancingData(void* lockedData, unsigned stride, unsigned& freeIndex);
    /// Prepare and draw.
    void Draw(View* view, Camera* camera, bool allowDepthWrite) const;

    /// Instance data.
    PODVector<InstanceData> instances_;
    /// Instance stream start index, or M_MAX_UNSIGNED if transforms not pre-set.
    unsigned startIndex_;
};

/// Instanced draw call grouping key.
struct BatchGroupKey
{
    /// Construct undefined.
    BatchGroupKey() = default;

    /// Construct from a batch.
    explicit BatchGroupKey(const Batch& batch) :
        zone_(batch.zone_),
        lightQueue_(batch.lightQueue_),
        pass_(batch.pass_),
        material_(batch.material_),
        geometry_(batch.geometry_),
        renderOrder_(batch.renderOrder_)
    {
    }

    /// Zone.
    Zone* zone_;
    /// Light properties.
    LightBatchQueue* lightQueue_;
    /// Material pass.
    Pass* pass_;
    /// Material.
    Material* material_;
    /// Geometry.
    Geometry* geometry_;
    /// 8-bit render order modifier from material.
    unsigned char renderOrder_;

    /// Test for equality with another batch group key.
    bool operator ==(const BatchGroupKey& rhs) const
    {
        return zone_ == rhs.zone_ && lightQueue_ == rhs.lightQueue_ && pass_ == rhs.pass_ && material_ == rhs.material_ &&
               geometry_ == rhs.geometry_ && renderOrder_ == rhs.renderOrder_;
    }

    /// Test for inequality with another batch group key.
    bool operator !=(const BatchGroupKey& rhs) const
    {
        return zone_ != rhs.zone_ || lightQueue_ != rhs.lightQueue_ || pass_ != rhs.pass_ || material_ != rhs.material_ ||
               geometry_ != rhs.geometry_ || renderOrder_ != rhs.renderOrder_;
    }

    /// Return hash value.
    unsigned ToHash() const;
};

/// Queue that contains both instanced and non-instanced draw calls.
struct BatchQueue
{
public:
    /// Clear for new frame by clearing all groups and batches.
    void Clear(int maxSortedInstances);
    /// Sort non-instanced draw calls back to front.
    void SortBackToFront();
    /// Sort instanced and non-instanced draw calls front to back.
    void SortFrontToBack();
    /// Sort batches front to back while also maintaining state sorting.
    void SortFrontToBack2Pass(PODVector<Batch*>& batches);
    /// Pre-set instance data of all groups. The vertex buffer must be big enough to hold all data.
    void SetInstancingData(void* lockedData, unsigned stride, unsigned& freeIndex);
    /// Draw.
    void Draw(View* view, Camera* camera, bool markToStencil, bool usingLightOptimization, bool allowDepthWrite) const;
    /// Return the combined amount of instances.
    unsigned GetNumInstances() const;

    /// Return whether the batch group is empty.
    bool IsEmpty() const { return batches_.Empty() && batchGroups_.Empty(); }

    /// Instanced draw calls.
    HashMap<BatchGroupKey, BatchGroup> batchGroups_; // 未排序的实例批次（根据场景中相机可见的几何体生成）
    /// Shader remapping table for 2-pass state and distance sort.
    HashMap<unsigned, unsigned> shaderRemapping_;
    /// Material remapping table for 2-pass state and distance sort.
    HashMap<unsigned short, unsigned short> materialRemapping_;
    /// Geometry remapping table for 2-pass state and distance sort.
    HashMap<unsigned short, unsigned short> geometryRemapping_;

    /// Unsorted non-instanced draw calls.
    PODVector<Batch> batches_; // 未排序的批次（根据场景中相机可见的几何体生成）
    /// Sorted non-instanced draw calls.
    PODVector<Batch*> sortedBatches_; // 排序后的批次（根据batches_生成）
    /// Sorted instanced draw calls.
    PODVector<BatchGroup*> sortedBatchGroups_; // 排序后的实例批次（根据batchGroups_生成）
    /// Maximum sorted instances.
    unsigned maxSortedInstances_;
    // 以下五项从RenderPath中产生（vsdefines=、psdefines=）
    /// Whether the pass command contains extra shader defines.
    bool hasExtraDefines_;
    /// Vertex shader extra defines.
    String vsExtraDefines_; // RenderPath中<command vsdefines="" />
    /// Pixel shader extra defines.
    String psExtraDefines_;
    /// Hash for vertex shader extra defines.
    StringHash vsExtraDefinesHash_;
    /// Hash for pixel shader extra defines.
    StringHash psExtraDefinesHash_;
};

// 用于生成阴影深度图的批次
/// Queue for shadow map draw calls
struct ShadowBatchQueue
{
    /// Shadow map camera.
    Camera* shadowCamera_{};
    /// Shadow map viewport.
    IntRect shadowViewport_;
    /// Shadow caster draw calls.
    BatchQueue shadowBatches_; // 用于投射阴影的几何体批次
    /// Directional light cascade near split distance.
    float nearSplit_{};
    /// Directional light cascade far split distance.
    float farSplit_{};
};

/// Queue for light related draw calls.
struct LightBatchQueue // 对于逐像素光源（light_指向该光源），保存几何体的光照批次（litBaseBatches_、litBatches_）、阴影批次（shadowSplits_）、光源本体批次（volumeBatches_）、阴影深度图（shadowMap_）；对于逐顶点光源，只保存顶点光列表（vertexLights_，引用自Drawable::vertexLights_），其他成员为空
{
    /// Per-pixel light.
    Light* light_; // 像素光
    /// Light negative flag.
    bool negative_; // 灯光是否为负（变暗）颜色
    /// Shadow map depth texture.
    Texture2D* shadowMap_; // 阴影深度贴图
    /// Lit geometry draw calls, base (replace blend mode)
    BatchQueue litBaseBatches_; // 像素光的基础批次，如果light_是几何体的第一个像素光源，则批次就放到这里
    /// Lit geometry draw calls, non-base (additive)
    BatchQueue litBatches_; // 像素光的附加批次，如果light_是几何体的后续像素光源（不是第一个），则批次就放到这里
    /// Shadow map split queues.
    Vector<ShadowBatchQueue> shadowSplits_; // 各投影相机（阴影层级）的几何体（可投射阴影）批次（用于产生阴影深度图）
    /// Per-vertex lights.
    PODVector<Light*> vertexLights_; // 顶点光列表，此时light_为空
    /// Light volume draw calls.
    PODVector<Batch> volumeBatches_; // 如果是延迟渲染，则保存本光源的几何体批次数据（则通过该变量进入批次（volumeBatches_[].lightQueue_））
};

}
