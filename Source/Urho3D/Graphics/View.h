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

#include "../Container/HashSet.h"
#include "../Container/List.h"
#include "../Core/Object.h"
#include "../Graphics/Batch.h"
#include "../Graphics/Light.h"
#include "../Graphics/Zone.h"
#include "../Math/Polyhedron.h"

namespace Urho3D
{

class Camera;
class DebugRenderer;
class Light;
class Drawable;
class Graphics;
class OcclusionBuffer;
class Octree;
class Renderer;
class RenderPath;
class RenderSurface;
class Technique;
class Texture;
class Texture2D;
class Viewport;
class Zone;
struct RenderPathCommand;
struct WorkItem;

/// Intermediate light processing result.
struct LightQueryResult
{
    /// Light.
    Light* light_;
    /// Lit geometries.
    PODVector<Drawable*> litGeometries_; // 被本light_照亮的几何体（观察矩阵内）
    /// Shadow casters.
    PODVector<Drawable*> shadowCasters_; // 影子投射器（能产生阴影的几何体）
    /// Shadow cameras.
    Camera* shadowCameras_[MAX_LIGHT_SPLITS]; // 每个LOD的阴影相机，用于对该区域产生阴影贴图（深度图）
    /// Shadow caster start indices.
    unsigned shadowCasterBegin_[MAX_LIGHT_SPLITS]; // 每个LOD区域中投射阴影的几何体的开始id（对应shadowCasters_的索引）
    /// Shadow caster end indices.
    unsigned shadowCasterEnd_[MAX_LIGHT_SPLITS]; // 每个LOD区域中投射阴影的几何体的结束id（对应shadowCasters_的索引）
    /// Combined bounding box of shadow casters in light projection space. Only used for focused spot lights.
    BoundingBox shadowCasterBox_[MAX_LIGHT_SPLITS]; // 阴影投射器在灯光投影空间的组合边界框。仅用于聚光灯。
    /// Shadow camera near splits (directional lights only.)
    float shadowNearSplits_[MAX_LIGHT_SPLITS]; // 每个阴影LOD区域的近值
    /// Shadow camera far splits (directional lights only.)
    float shadowFarSplits_[MAX_LIGHT_SPLITS]; // 每个阴影LOD区域的远值
    /// Shadow map split count.
    unsigned numSplits_; // 阴影的LOD划分，https://blog.csdn.net/qq_29523119/article/details/79266293
};
// 在实时渲染中，存在这样的一个渲染优化手段，也就是“LOD”,level of details,也就是细节等级的意思，总体来说也就是根据我们观察相机距离被观察物体的距离来决定优化被观察物体的渲染细节。
// CascadeShadowMap其实就是来源于LOD的思想，总体的原理: 将我们观察相机的视截体根据与相机空间的原点的远近距离来划分为多个部分(也就是所谓的层级Cascade)，并将这几个部分的物体渲染到相应的ShadowMap。

/// Scene render pass info.
struct ScenePassInfo
{
    /// Pass index.
    unsigned passIndex_;
    /// Allow instancing flag.
    bool allowInstancing_;
    /// Mark to stencil flag.
    bool markToStencil_;
    /// Vertex light flag.
    bool vertexLights_;
    /// Batch queue.
    BatchQueue* batchQueue_;
};

/// Per-thread geometry, light and scene range collection structure.
struct PerThreadSceneResult
{
    /// Geometry objects.
    PODVector<Drawable*> geometries_;
    /// Lights.
    PODVector<Light*> lights_;
    /// Scene minimum Z value.
    float minZ_; // 观察空间的深度值
    /// Scene maximum Z value.
    float maxZ_; // 观察空间的深度值
};

static const unsigned MAX_VIEWPORT_TEXTURES = 2;

/// Internal structure for 3D rendering work. Created for each backbuffer and texture viewport, but not for shadow cameras.
class URHO3D_API View : public Object
{
    friend void CheckVisibilityWork(const WorkItem* item, unsigned threadIndex);
    friend void ProcessLightWork(const WorkItem* item, unsigned threadIndex);

    URHO3D_OBJECT(View, Object);

public:
    /// Construct.
    explicit View(Context* context);
    /// Destruct.
    ~View() override = default;

    /// Define with rendertarget and viewport. Return true if successful.
    bool Define(RenderSurface* renderTarget, Viewport* viewport);
    /// Update and cull objects and construct rendering batches.
    void Update(const FrameInfo& frame);
    /// Render batches.
    void Render();

    /// Return graphics subsystem.
    Graphics* GetGraphics() const;
    /// Return renderer subsystem.
    Renderer* GetRenderer() const;

    /// Return scene.
    Scene* GetScene() const { return scene_; }

    /// Return octree.
    Octree* GetOctree() const { return octree_; }

    /// Return viewport camera.
    Camera* GetCamera() const { return camera_; }

    /// Return culling camera. Normally same as the viewport camera.
    Camera* GetCullCamera() const { return cullCamera_; }

    /// Return information of the frame being rendered.
    const FrameInfo& GetFrameInfo() const { return frame_; }

    /// Return the rendertarget. 0 if using the backbuffer.
    RenderSurface* GetRenderTarget() const { return renderTarget_; }

    /// Return whether should draw debug geometry.
    bool GetDrawDebug() const { return drawDebug_; }

    /// Return view rectangle.
    const IntRect& GetViewRect() const { return viewRect_; }

    /// Return view dimensions.
    const IntVector2& GetViewSize() const { return viewSize_; }

    /// Return geometry objects.
    const PODVector<Drawable*>& GetGeometries() const { return geometries_; }

    /// Return occluder objects.
    const PODVector<Drawable*>& GetOccluders() const { return occluders_; }

    /// Return lights.
    const PODVector<Light*>& GetLights() const { return lights_; }

    /// Return light batch queues.
    const Vector<LightBatchQueue>& GetLightQueues() const { return lightQueues_; }

    /// Return the last used software occlusion buffer.
    OcclusionBuffer* GetOcclusionBuffer() const { return occlusionBuffer_; }

    /// Return number of occluders that were actually rendered. Occluders may be rejected if running out of triangles or if behind other occluders.
    unsigned GetNumActiveOccluders() const { return activeOccluders_; }

    /// Return the source view that was already prepared. Used when viewports specify the same culling camera.
    View* GetSourceView() const;

    /// Set global (per-frame) shader parameters. Called by Batch and internally by View.
    void SetGlobalShaderParameters();
    /// Set camera-specific shader parameters. Called by Batch and internally by View.
    void SetCameraShaderParameters(Camera* camera);
    /// Set command's shader parameters if any. Called internally by View.
    void SetCommandShaderParameters(const RenderPathCommand& command);
    /// Set G-buffer offset and inverse size shader parameters. Called by Batch and internally by View.
    void SetGBufferShaderParameters(const IntVector2& texSize, const IntRect& viewRect);

    /// Draw a fullscreen quad. Shaders and renderstates must have been set beforehand. Quad will be drawn to the middle of depth range, similarly to deferred directional lights.
    void DrawFullscreenQuad(bool setIdentityProjection = false);

    /// Get a named texture from the rendertarget list or from the resource cache, to be either used as a rendertarget or texture binding.
    Texture* FindNamedTexture(const String& name, bool isRenderTarget, bool isVolumeMap = false);

private:
    /// Query the octree for drawable objects.
    void GetDrawables();
    /// Construct batches from the drawable objects.
    void GetBatches();
    /// Get lit geometries and shadowcasters for visible lights.
    void ProcessLights();
    /// Get batches from lit geometries and shadowcasters.
    void GetLightBatches();
    /// Get unlit batches.
    void GetBaseBatches();
    /// Update geometries and sort batches.
    void UpdateGeometries();
    /// Get pixel lit batches for a certain light and drawable.
    void GetLitBatches(Drawable* drawable, LightBatchQueue& lightQueue, BatchQueue* alphaQueue);
    /// Execute render commands.
    void ExecuteRenderPathCommands();
    /// Set rendertargets for current render command.
    void SetRenderTargets(RenderPathCommand& command);
    /// Set textures for current render command. Return whether depth write is allowed (depth-stencil not bound as a texture.)
    bool SetTextures(RenderPathCommand& command);
    /// Perform a quad rendering command.
    void RenderQuad(RenderPathCommand& command);
    /// Check if a command is enabled and has content to render. To be called only after render update has completed for the frame.
    bool IsNecessary(const RenderPathCommand& command);
    /// Check if a command reads the destination render target.
    bool CheckViewportRead(const RenderPathCommand& command);
    /// Check if a command writes into the destination render target.
    bool CheckViewportWrite(const RenderPathCommand& command);
    /// Check whether a command should use pingponging instead of resolve from destination render target to viewport texture.
    bool CheckPingpong(unsigned index);
    /// Allocate needed screen buffers.
    void AllocateScreenBuffers();
    /// Blit the viewport from one surface to another.
    void BlitFramebuffer(Texture* source, RenderSurface* destination, bool depthWrite);
    /// Query for occluders as seen from a camera.
    void UpdateOccluders(PODVector<Drawable*>& occluders, Camera* camera);
    /// Draw occluders to occlusion buffer.
    void DrawOccluders(OcclusionBuffer* buffer, const PODVector<Drawable*>& occluders);
    /// Query for lit geometries and shadow casters for a light.
    void ProcessLight(LightQueryResult& query, unsigned threadIndex);
    /// Process shadow casters' visibilities and build their combined view- or projection-space bounding box.
    void ProcessShadowCasters(LightQueryResult& query, const PODVector<Drawable*>& drawables, unsigned splitIndex);
    /// Set up initial shadow camera view(s).
    void SetupShadowCameras(LightQueryResult& query);
    /// Set up a directional light shadow camera
    void SetupDirLightShadowCamera(Camera* shadowCamera, Light* light, float nearSplit, float farSplit);
    /// Finalize shadow camera view after shadow casters and the shadow map are known.
    void
        FinalizeShadowCamera(Camera* shadowCamera, Light* light, const IntRect& shadowViewport, const BoundingBox& shadowCasterBox);
    /// Quantize a directional light shadow camera view to eliminate swimming.
    void
        QuantizeDirLightShadowCamera(Camera* shadowCamera, Light* light, const IntRect& shadowViewport, const BoundingBox& viewBox);
    /// Check visibility of one shadow caster.
    bool IsShadowCasterVisible(Drawable* drawable, BoundingBox lightViewBox, Camera* shadowCamera, const Matrix3x4& lightView,
        const Frustum& lightViewFrustum, const BoundingBox& lightViewFrustumBox);
    /// Return the viewport for a shadow map split.
    IntRect GetShadowMapViewport(Light* light, int splitIndex, Texture2D* shadowMap);
    /// Find and set a new zone for a drawable when it has moved.
    void FindZone(Drawable* drawable);
    /// Return material technique, considering the drawable's LOD distance.
    Technique* GetTechnique(Drawable* drawable, Material* material);
    /// Check if material should render an auxiliary view (if it has a camera attached.)
    void CheckMaterialForAuxView(Material* material);
    /// Set shader defines for a batch queue if used.
    void SetQueueShaderDefines(BatchQueue& queue, const RenderPathCommand& command);
    /// Choose shaders for a batch and add it to queue.
    void AddBatchToQueue(BatchQueue& queue, Batch& batch, Technique* tech, bool allowInstancing = true, bool allowShadows = true);
    /// Prepare instancing buffer by filling it with all instance transforms.
    void PrepareInstancingBuffer();
    /// Set up a light volume rendering batch.
    void SetupLightVolumeBatch(Batch& batch);
    /// Check whether a light queue needs shadow rendering.
    bool NeedRenderShadowMap(const LightBatchQueue& queue);
    /// Render a shadow map.
    void RenderShadowMap(const LightBatchQueue& queue);
    /// Return the proper depth-stencil surface to use for a rendertarget.
    RenderSurface* GetDepthStencil(RenderSurface* renderTarget);
    /// Helper function to get the render surface from a texture. 2D textures will always return the first face only.
    RenderSurface* GetRenderSurfaceFromTexture(Texture* texture, CubeMapFace face = FACE_POSITIVE_X);
    /// Send a view update or render related event through the Renderer subsystem. The parameters are the same for all of them.
    void SendViewEvent(StringHash eventType);

    /// Return the drawable's zone, or camera zone if it has override mode enabled.
    Zone* GetZone(Drawable* drawable)
    {
        if (cameraZoneOverride_)
            return cameraZone_;
        Zone* drawableZone = drawable->GetZone();
        return drawableZone ? drawableZone : cameraZone_;
    }

    /// Return the drawable's light mask, considering also its zone.
    unsigned GetLightMask(Drawable* drawable)
    {
        return drawable->GetLightMask() & GetZone(drawable)->GetLightMask();
    }

    /// Return the drawable's shadow mask, considering also its zone.
    unsigned GetShadowMask(Drawable* drawable)
    {
        return drawable->GetShadowMask() & GetZone(drawable)->GetShadowMask();
    }

    /// Return hash code for a vertex light queue.
    unsigned long long GetVertexLightQueueHash(const PODVector<Light*>& vertexLights)
    {
        unsigned long long hash = 0;
        for (PODVector<Light*>::ConstIterator i = vertexLights.Begin(); i != vertexLights.End(); ++i)
            hash += (unsigned long long)(*i);
        return hash;
    }

    /// Graphics subsystem.
    WeakPtr<Graphics> graphics_;
    /// Renderer subsystem.
    WeakPtr<Renderer> renderer_;
    /// Scene to use.
    Scene* scene_{};
    /// Octree to use.
    Octree* octree_{};
    /// Viewport (rendering) camera.
    Camera* camera_{};
    /// Culling camera. Usually same as the viewport camera.
    Camera* cullCamera_{}; // 立体渲染时，两个渲染相机（模拟双眼），将其中一个设为裁剪相机（提供同一可见场景给渲染相机）
    /// Shared source view. Null if this view is using its own culling.
    WeakPtr<View> sourceView_; // 立体渲染时，cullCamera_对应的View
    /// Zone the camera is inside, or default zone if not assigned.
    Zone* cameraZone_{}; // 相机（cullCamera_）所在的优先级最高的区域，如果没分配就是缺省区域
    /// Zone at far clip plane.
    Zone* farClipZone_{}; // 相机（cullCamera_）远裁剪面中心点所在的优先级最高的区域；如果没分配或者cameraZone_为覆盖模式，就设为cameraZone_
    /// Occlusion buffer for the main camera.
    OcclusionBuffer* occlusionBuffer_{}; // 根据遮挡体生成，用于可见物剔除
    /// Destination color rendertarget.
    RenderSurface* renderTarget_{}; // 目标RT，Define()中定义
    /// Substitute rendertarget for deferred rendering. Allocated if necessary.
    RenderSurface* substituteRenderTarget_{};
    /// Texture(s) for sampling the viewport contents. Allocated if necessary.
    Texture* viewportTextures_[MAX_VIEWPORT_TEXTURES]{};
    /// Color rendertarget active for the current renderpath command.
    RenderSurface* currentRenderTarget_{};
    /// Last used custom depth render surface.
    RenderSurface* lastCustomDepthSurface_{};
    /// Texture containing the latest viewport texture.
    Texture* currentViewportTexture_{}; // 从当前视口取得的纹理
    /// Dummy texture for D3D9 depth only rendering.
    Texture* depthOnlyDummyTexture_{};
    /// Viewport rectangle.
    IntRect viewRect_;
    /// Viewport size.
    IntVector2 viewSize_; // 视口尺寸
    /// Destination rendertarget size.
    IntVector2 rtSize_; // 渲染目标（rendertarget）尺寸
    /// Information of the frame being rendered.
    FrameInfo frame_{};
    /// View aspect ratio.
    float aspectRatio_{};
    /// Minimum Z value of the visible scene.
    float minZ_{};
    /// Maximum Z value of the visible scene.
    float maxZ_{};
    /// Material quality level.
    int materialQuality_{};
    /// Maximum number of occluder triangles.
    int maxOccluderTriangles_{};
    /// Minimum number of instances required in a batch group to render as instanced.
    int minInstances_{};
    /// Highest zone priority currently visible.
    int highestZonePriority_{}; // 相机可见区域的优先级（Zone::GetPriority()）的最大值
    /// Geometries updated flag.
    bool geometriesUpdated_{};
    /// Camera zone's override flag.
    bool cameraZoneOverride_{};
    /// Draw shadows flag.
    bool drawShadows_{};
    /// Deferred flag. Inferred from the existence of a light volume command in the renderpath.
    bool deferred_{};
    /// Deferred ambient pass flag. This means that the destination rendertarget is being written to at the same time as albedo/normal/depth buffers, and needs to be RGBA on OpenGL.
    bool deferredAmbient_{};
    /// Forward light base pass optimization flag. If in use, combine the base pass and first light for all opaque objects.
    bool useLitBase_{};
    /// Has scene passes flag. If no scene passes, view can be defined without a valid scene or camera to only perform quad rendering.
    bool hasScenePasses_{};
    /// Whether is using a custom readable depth texture without a stencil channel.
    bool noStencil_{}; // 是否使用不带模板通道的自定义可读深度纹理。
    /// Draw debug geometry flag. Copied from the viewport.
    bool drawDebug_{};
    /// Renderpath.
    RenderPath* renderPath_{};
    /// Per-thread octree query results.
    Vector<PODVector<Drawable*> > tempDrawables_;
    /// Per-thread geometries, lights and Z range collection results.
    Vector<PerThreadSceneResult> sceneResults_; // 用于多线程收集Drawable、Light，以及确定观察空间中的深度范围
    /// Visible zones.
    PODVector<Zone*> zones_; // 相机（cullCamera_）可见的区域
    /// Visible geometry objects.
    PODVector<Drawable*> geometries_; // 相机可见的几何体（DRAWABLE_GEOMETRY）
    /// Geometry objects that will be updated in the main thread.
    PODVector<Drawable*> nonThreadedGeometries_; // 在主线程中更新的几何体
    /// Geometry objects that will be updated in worker threads.
    PODVector<Drawable*> threadedGeometries_; // 在工作线程中更新的几何体
    /// Occluder objects.
    PODVector<Drawable*> occluders_; // 相机（cullCamera_）可见的遮挡物（Drawable::occluder_为真）
    /// Lights.
    PODVector<Light*> lights_; // 相机可见的光源（DRAWABLE_LIGHT）
    /// Number of active occluders.
    unsigned activeOccluders_{};

    /// Drawables that limit their maximum light count.
    HashSet<Drawable*> maxLightsDrawables_;
    /// Rendertargets defined by the renderpath.
    HashMap<StringHash, Texture*> renderTargets_;
    /// Intermediate light processing results.
    Vector<LightQueryResult> lightQueryResults_; // 与各光源关联的几何体及阴影信息（lights_中每个光源及其影响的几何体为一组）
    /// Info for scene render passes defined by the renderpath.
    PODVector<ScenePassInfo> scenePasses_; // 保存renderpath中command type="scenepass"的各个pass信息
    /// Per-pixel light queues.
    Vector<LightBatchQueue> lightQueues_; // 逐像素光源的批次队列（受lightQueues_[x].light_影响的Drawable，生成Batch，放入lightQueues_[x]）
    /// Per-vertex light queues.
    HashMap<unsigned long long, LightBatchQueue> vertexLightQueues_; // 逐顶点光源的批次队列（只受顶点）
    /// Batch queues by pass index.
    HashMap<unsigned, BatchQueue> batchQueues_; // command type="scenepass" 的各个pass的批次（不受光照影响）
    /// Index of the GBuffer pass.
    unsigned gBufferPassIndex_{};
    /// Index of the opaque forward base pass.
    unsigned basePassIndex_{}; // pass name="base"
    /// Index of the alpha pass.
    unsigned alphaPassIndex_{};
    /// Index of the forward light pass.
    unsigned lightPassIndex_{}; // pass name="light"
    /// Index of the litbase pass.
    unsigned litBasePassIndex_{}; // pass name="litbase"
    /// Index of the litalpha pass.
    unsigned litAlphaPassIndex_{};
    /// Pointer to the light volume command if any.
    const RenderPathCommand* lightVolumeCommand_{};
    /// Pointer to the forwardlights command if any.
    const RenderPathCommand* forwardLightsCommand_{};
    /// Pointer to the current commmand if it contains shader parameters to be set for a render pass.
    const RenderPathCommand* passCommand_{};
    /// Flag for scene being resolved from the backbuffer.
    bool usedResolve_{};
};

// viewport的渲染步骤大致如下：
// 1，在八叉树中查询相机视锥中的灯光和可见对象。
// 2，检查每个可见光对物体的影响。如果灯光投射阴影，请在八叉树中查询shadowcaster对象。
// 3，根据RenderPath中的场景过程，为可见对象构造渲染操作（batches）。
// 4，在帧末尾的渲染步骤中执行RenderPath命令序列。
// 5，如果场景具有DebugRenderer组件，并且viewport（视口）启用了调试渲染，则最后渲染调试几何体。可使用SetDrawDebug（）进行控制，默认为启用。

// 在默认渲染路径中，渲染操作按以下顺序进行：
// 1，不透明几何体环境光过程，或延迟渲染模式下的G缓冲区过程。
// 2，不透明几何体每像素光照过程。对于阴影投射灯光，首先渲染阴影贴图。
// 3，（仅限Light pre-pass）不透明几何体材质过程，使用累积的每像素照明渲染对象。
// 4，为了自定义渲染顺序的Post-opaque pass，比如skybox。
// 5，折射几何体过程。
// 6，透明几何体过程。透明的、alpha混合的对象将根据距离排序并从后到前进行渲染，以确保正确的混合。
// 7，Post-alpha pass，可用于3D叠加，应出现在一切其他的顶层。

// 以下技术将用于减少渲染时CPU和GPU的工作量。默认情况下，它们都处于启用状态：
// 1，软件光栅化遮挡（Software rasterized occlusion）：八叉树查询到可见对象后，标记为遮挡器的对象在CPU上渲染到一个小的层次深度缓冲区，并用于测试非遮挡器的可见性。使用SetMaxOccluderTriangles（）和SetOccluderSizeThreshold（）配置遮挡渲染。遮挡测试将始终是多线程的，但是默认情况下遮挡渲染是单线程的，以便在front-to-back渲染时拒绝后续遮挡器。。使用SetThreadedOcclusion（）也可以在渲染中启用线程，但这实际上在地形场景（例如，地形面片充当遮挡器）中的性能会更差。
// 2，硬件实例（Hardware instancing）：具有相同几何体、材质和灯光的渲染操作将组合在一起并作为一个draw call执行（如果支持）。请注意，即使实例不可用，它们仍然可以从分组中获益，因为渲染状态只需在渲染每个组之前检查和设置一次，从而降低了CPU成本。
// 3，灯光模板遮罩（Light stencil masking）：在正向渲染中，在附加重新渲染由聚光灯或点光源照亮的对象之前，灯光的边界形状将渲染到模板缓冲区，以确保不处理灯光范围之外的像素。
// 请注意，在内容级别可以有更多的优化机会，例如使用几何体和材质LOD，将许多静态对象分组到一个对象中以减少绘制调用，最小化每个对象的子几何体（子网格）数量以减少绘制调用，使用纹理贴图集避免渲染状态更改，使用压缩（和较小）纹理，并设置对象、灯光和阴影的最大绘制距离。

// pingpong：
// 在两个模块间交换数据时，上一级处理的结果不能马上被下一级所处理完成，这样上一级必须等待下一级处理完成才可以送新的数据，这样就会对性能产生很大的损失。
// 引入pingpong后我们可以不去等待下一级处理结束，而是将结果保存在pong路的缓存中，pong路的数据准备好的时刻，ping路的数据也处理完毕（下一级），然后无需等待直接处理pong路数据，上一级也无需等待，转而将结果存储在ping路。这样便提高了处理效率。
// 所谓ping-pong buffer，也就是定义两个buffer，当有数据进来的时候，负责写入buffer的进程就寻找第一个没有被占用而且可写的buffer，进行写入，写好之后，将占用flag释放，同时设置一个flag提示此buffer已经可读，然后再接下去找另外一个可写的buffer，写入新的数据。而读入的进程也是一直对buffer状态进行检测，一旦发现没有被占用，而且已经可以被读，就把这个buffer的数据取出来，然后标志为可写。

}
