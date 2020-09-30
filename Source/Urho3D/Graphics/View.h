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
    PODVector<Drawable*> litGeometries_; // ����light_�����ļ����壨�۲�����ڣ�
    /// Shadow casters.
    PODVector<Drawable*> shadowCasters_; // Ӱ��Ͷ�������ܲ�����Ӱ�ļ����壩
    /// Shadow cameras.
    Camera* shadowCameras_[MAX_LIGHT_SPLITS]; // ÿ��LOD����Ӱ��������ڶԸ����������Ӱ��ͼ�����ͼ��
    /// Shadow caster start indices.
    unsigned shadowCasterBegin_[MAX_LIGHT_SPLITS]; // ÿ��LOD������Ͷ����Ӱ�ļ�����Ŀ�ʼid����ӦshadowCasters_��������
    /// Shadow caster end indices.
    unsigned shadowCasterEnd_[MAX_LIGHT_SPLITS]; // ÿ��LOD������Ͷ����Ӱ�ļ�����Ľ���id����ӦshadowCasters_��������
    /// Combined bounding box of shadow casters in light projection space. Only used for focused spot lights.
    BoundingBox shadowCasterBox_[MAX_LIGHT_SPLITS]; // ��ӰͶ�����ڵƹ�ͶӰ�ռ����ϱ߽�򡣽����ھ۹�ơ�
    /// Shadow camera near splits (directional lights only.)
    float shadowNearSplits_[MAX_LIGHT_SPLITS]; // ÿ����ӰLOD����Ľ�ֵ
    /// Shadow camera far splits (directional lights only.)
    float shadowFarSplits_[MAX_LIGHT_SPLITS]; // ÿ����ӰLOD�����Զֵ
    /// Shadow map split count.
    unsigned numSplits_; // ��Ӱ��LOD���֣�https://blog.csdn.net/qq_29523119/article/details/79266293
};
// ��ʵʱ��Ⱦ�У�����������һ����Ⱦ�Ż��ֶΣ�Ҳ���ǡ�LOD��,level of details,Ҳ����ϸ�ڵȼ�����˼��������˵Ҳ���Ǹ������ǹ۲�������뱻�۲�����ľ����������Ż����۲��������Ⱦϸ�ڡ�
// CascadeShadowMap��ʵ������Դ��LOD��˼�룬�����ԭ��: �����ǹ۲�������ӽ������������ռ��ԭ���Զ������������Ϊ�������(Ҳ������ν�Ĳ㼶Cascade)�������⼸�����ֵ�������Ⱦ����Ӧ��ShadowMap��

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
    float minZ_; // �۲�ռ�����ֵ
    /// Scene maximum Z value.
    float maxZ_; // �۲�ռ�����ֵ
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
    Camera* cullCamera_{}; // ������Ⱦʱ��������Ⱦ�����ģ��˫�ۣ���������һ����Ϊ�ü�������ṩͬһ�ɼ���������Ⱦ�����
    /// Shared source view. Null if this view is using its own culling.
    WeakPtr<View> sourceView_; // ������Ⱦʱ��cullCamera_��Ӧ��View
    /// Zone the camera is inside, or default zone if not assigned.
    Zone* cameraZone_{}; // �����cullCamera_�����ڵ����ȼ���ߵ��������û�������ȱʡ����
    /// Zone at far clip plane.
    Zone* farClipZone_{}; // �����cullCamera_��Զ�ü������ĵ����ڵ����ȼ���ߵ��������û�������cameraZone_Ϊ����ģʽ������ΪcameraZone_
    /// Occlusion buffer for the main camera.
    OcclusionBuffer* occlusionBuffer_{}; // �����ڵ������ɣ����ڿɼ����޳�
    /// Destination color rendertarget.
    RenderSurface* renderTarget_{}; // Ŀ��RT��Define()�ж���
    /// Substitute rendertarget for deferred rendering. Allocated if necessary.
    RenderSurface* substituteRenderTarget_{};
    /// Texture(s) for sampling the viewport contents. Allocated if necessary.
    Texture* viewportTextures_[MAX_VIEWPORT_TEXTURES]{};
    /// Color rendertarget active for the current renderpath command.
    RenderSurface* currentRenderTarget_{};
    /// Last used custom depth render surface.
    RenderSurface* lastCustomDepthSurface_{};
    /// Texture containing the latest viewport texture.
    Texture* currentViewportTexture_{}; // �ӵ�ǰ�ӿ�ȡ�õ�����
    /// Dummy texture for D3D9 depth only rendering.
    Texture* depthOnlyDummyTexture_{};
    /// Viewport rectangle.
    IntRect viewRect_;
    /// Viewport size.
    IntVector2 viewSize_; // �ӿڳߴ�
    /// Destination rendertarget size.
    IntVector2 rtSize_; // ��ȾĿ�꣨rendertarget���ߴ�
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
    int highestZonePriority_{}; // ����ɼ���������ȼ���Zone::GetPriority()�������ֵ
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
    bool noStencil_{}; // �Ƿ�ʹ�ò���ģ��ͨ�����Զ���ɶ��������
    /// Draw debug geometry flag. Copied from the viewport.
    bool drawDebug_{};
    /// Renderpath.
    RenderPath* renderPath_{};
    /// Per-thread octree query results.
    Vector<PODVector<Drawable*> > tempDrawables_;
    /// Per-thread geometries, lights and Z range collection results.
    Vector<PerThreadSceneResult> sceneResults_; // ���ڶ��߳��ռ�Drawable��Light���Լ�ȷ���۲�ռ��е���ȷ�Χ
    /// Visible zones.
    PODVector<Zone*> zones_; // �����cullCamera_���ɼ�������
    /// Visible geometry objects.
    PODVector<Drawable*> geometries_; // ����ɼ��ļ����壨DRAWABLE_GEOMETRY��
    /// Geometry objects that will be updated in the main thread.
    PODVector<Drawable*> nonThreadedGeometries_; // �����߳��и��µļ�����
    /// Geometry objects that will be updated in worker threads.
    PODVector<Drawable*> threadedGeometries_; // �ڹ����߳��и��µļ�����
    /// Occluder objects.
    PODVector<Drawable*> occluders_; // �����cullCamera_���ɼ����ڵ��Drawable::occluder_Ϊ�棩
    /// Lights.
    PODVector<Light*> lights_; // ����ɼ��Ĺ�Դ��DRAWABLE_LIGHT��
    /// Number of active occluders.
    unsigned activeOccluders_{};

    /// Drawables that limit their maximum light count.
    HashSet<Drawable*> maxLightsDrawables_;
    /// Rendertargets defined by the renderpath.
    HashMap<StringHash, Texture*> renderTargets_;
    /// Intermediate light processing results.
    Vector<LightQueryResult> lightQueryResults_; // �����Դ�����ļ����弰��Ӱ��Ϣ��lights_��ÿ����Դ����Ӱ��ļ�����Ϊһ�飩
    /// Info for scene render passes defined by the renderpath.
    PODVector<ScenePassInfo> scenePasses_; // ����renderpath��command type="scenepass"�ĸ���pass��Ϣ
    /// Per-pixel light queues.
    Vector<LightBatchQueue> lightQueues_; // �����ع�Դ�����ζ��У���lightQueues_[x].light_Ӱ���Drawable������Batch������lightQueues_[x]��
    /// Per-vertex light queues.
    HashMap<unsigned long long, LightBatchQueue> vertexLightQueues_; // �𶥵��Դ�����ζ��У�ֻ�ܶ��㣩
    /// Batch queues by pass index.
    HashMap<unsigned, BatchQueue> batchQueues_; // command type="scenepass" �ĸ���pass�����Σ����ܹ���Ӱ�죩
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

// viewport����Ⱦ����������£�
// 1���ڰ˲����в�ѯ�����׶�еĵƹ�Ϳɼ�����
// 2�����ÿ���ɼ���������Ӱ�졣����ƹ�Ͷ����Ӱ�����ڰ˲����в�ѯshadowcaster����
// 3������RenderPath�еĳ������̣�Ϊ�ɼ���������Ⱦ������batches����
// 4����֡ĩβ����Ⱦ������ִ��RenderPath�������С�
// 5�������������DebugRenderer���������viewport���ӿڣ������˵�����Ⱦ���������Ⱦ���Լ����塣��ʹ��SetDrawDebug�������п��ƣ�Ĭ��Ϊ���á�

// ��Ĭ����Ⱦ·���У���Ⱦ����������˳����У�
// 1����͸�������廷������̣����ӳ���Ⱦģʽ�µ�G���������̡�
// 2����͸��������ÿ���ع��չ��̡�������ӰͶ��ƹ⣬������Ⱦ��Ӱ��ͼ��
// 3��������Light pre-pass����͸����������ʹ��̣�ʹ���ۻ���ÿ����������Ⱦ����
// 4��Ϊ���Զ�����Ⱦ˳���Post-opaque pass������skybox��
// 5�����伸������̡�
// 6��͸����������̡�͸���ġ�alpha��ϵĶ��󽫸��ݾ������򲢴Ӻ�ǰ������Ⱦ����ȷ����ȷ�Ļ�ϡ�
// 7��Post-alpha pass��������3D���ӣ�Ӧ������һ�������Ķ��㡣

// ���¼��������ڼ�����ȾʱCPU��GPU�Ĺ�������Ĭ������£����Ƕ���������״̬��
// 1�������դ���ڵ���Software rasterized occlusion�����˲�����ѯ���ɼ�����󣬱��Ϊ�ڵ����Ķ�����CPU����Ⱦ��һ��С�Ĳ����Ȼ������������ڲ��Է��ڵ����Ŀɼ��ԡ�ʹ��SetMaxOccluderTriangles������SetOccluderSizeThreshold���������ڵ���Ⱦ���ڵ����Խ�ʼ���Ƕ��̵߳ģ�����Ĭ��������ڵ���Ⱦ�ǵ��̵߳ģ��Ա���front-to-back��Ⱦʱ�ܾ������ڵ�������ʹ��SetThreadedOcclusion����Ҳ��������Ⱦ�������̣߳�����ʵ�����ڵ��γ��������磬������Ƭ�䵱�ڵ������е����ܻ���
// 2��Ӳ��ʵ����Hardware instancing����������ͬ�����塢���ʺ͵ƹ����Ⱦ�����������һ����Ϊһ��draw callִ�У����֧�֣�����ע�⣬��ʹʵ�������ã�������Ȼ���Դӷ����л��棬��Ϊ��Ⱦ״ֻ̬������Ⱦÿ����֮ǰ��������һ�Σ��Ӷ�������CPU�ɱ���
// 3���ƹ�ģ�����֣�Light stencil masking������������Ⱦ�У��ڸ���������Ⱦ�ɾ۹�ƻ���Դ�����Ķ���֮ǰ���ƹ�ı߽���״����Ⱦ��ģ�建��������ȷ��������ƹⷶΧ֮������ء�
// ��ע�⣬�����ݼ�������и�����Ż����ᣬ����ʹ�ü�����Ͳ���LOD������ྲ̬������鵽һ���������Լ��ٻ��Ƶ��ã���С��ÿ��������Ӽ����壨�����������Լ��ٻ��Ƶ��ã�ʹ��������ͼ��������Ⱦ״̬���ģ�ʹ��ѹ�����ͽ�С�����������ö��󡢵ƹ����Ӱ�������ƾ��롣

// pingpong��
// ������ģ��佻������ʱ����һ������Ľ���������ϱ���һ����������ɣ�������һ������ȴ���һ��������ɲſ������µ����ݣ������ͻ�����ܲ����ܴ����ʧ��
// ����pingpong�����ǿ��Բ�ȥ�ȴ���һ��������������ǽ����������pong·�Ļ����У�pong·������׼���õ�ʱ�̣�ping·������Ҳ������ϣ���һ������Ȼ������ȴ�ֱ�Ӵ���pong·���ݣ���һ��Ҳ����ȴ���ת��������洢��ping·������������˴���Ч�ʡ�
// ��νping-pong buffer��Ҳ���Ƕ�������buffer���������ݽ�����ʱ�򣬸���д��buffer�Ľ��̾�Ѱ�ҵ�һ��û�б�ռ�ö��ҿ�д��buffer������д�룬д��֮�󣬽�ռ��flag�ͷţ�ͬʱ����һ��flag��ʾ��buffer�Ѿ��ɶ���Ȼ���ٽ���ȥ������һ����д��buffer��д���µ����ݡ�������Ľ���Ҳ��һֱ��buffer״̬���м�⣬һ������û�б�ռ�ã������Ѿ����Ա������Ͱ����buffer������ȡ������Ȼ���־Ϊ��д��

}
