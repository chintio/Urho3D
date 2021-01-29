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
#include "../Container/Ptr.h"
#include "../Graphics/GraphicsDefs.h"
#include "../Graphics/Skeleton.h"
#include "../Math/BoundingBox.h"
#include "../Resource/Resource.h"

namespace Urho3D
{

class Geometry;
class IndexBuffer;
class Graphics;
class VertexBuffer;

/// Vertex buffer morph data.
struct VertexBufferMorph
{
    /// Vertex elements.
    VertexMaskFlags elementMask_;
    /// Number of vertices.
    unsigned vertexCount_;
    /// Morphed vertices data size as bytes.
    unsigned dataSize_;
    /// Morphed vertices. Stored packed as <index, data> pairs.
    SharedArrayPtr<unsigned char> morphData_; // 要混合的顶点，格式为<index, data>，index指向Model.vertexBuffers_中的顶点元素索引（从0开始）
};

/// Definition of a model's vertex morph.
struct ModelMorph
{
    /// Morph name.
    String name_; // 变形动画的名称
    /// Morph name hash.
    StringHash nameHash_;
    /// Current morph weight.
    float weight_; // 混合权重
    /// Morph data per vertex buffer.
    HashMap<unsigned, VertexBufferMorph> buffers_; // 要混合的顶点信息，索引对应Model.vertexBuffers_索引
};

/// Description of vertex buffer data for asynchronous loading.
struct VertexBufferDesc
{
    /// Vertex count.
    unsigned vertexCount_;
    /// Vertex declaration.
    PODVector<VertexElement> vertexElements_;
    /// Vertex data size.
    unsigned dataSize_;
    /// Vertex data.
    SharedArrayPtr<unsigned char> data_;
};

/// Description of index buffer data for asynchronous loading.
struct IndexBufferDesc
{
    /// Index count.
    unsigned indexCount_;
    /// Index size.
    unsigned indexSize_;
    /// Index data size.
    unsigned dataSize_;
    /// Index data.
    SharedArrayPtr<unsigned char> data_;
};

/// Description of a geometry for asynchronous loading.
struct GeometryDesc // 异步加载的几何体描述
{
    /// Primitive type.
    PrimitiveType type_; // 图元类型（PrimitiveType）
    /// Vertex buffer ref.
    unsigned vbRef_; // 引用第几块顶点数据
    /// Index buffer ref.
    unsigned ibRef_; // 引用第几块索引数据
    /// Index start.
    unsigned indexStart_; // 索引开始位置
    /// Index count.
    unsigned indexCount_; // 索引个数
};

/// 3D model resource.
class URHO3D_API Model : public ResourceWithMetadata
{
    URHO3D_OBJECT(Model, ResourceWithMetadata);

public:
    /// Construct.
    explicit Model(Context* context);
    /// Destruct.
    ~Model() override;
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;
    /// Finish resource loading. Always called from the main thread. Return true if successful.
    bool EndLoad() override;
    /// Save resource. Return true if successful.
    bool Save(Serializer& dest) const override;

    /// Set local-space bounding box.
    void SetBoundingBox(const BoundingBox& box);
    /// Set vertex buffers and their morph ranges.
    bool SetVertexBuffers(const Vector<SharedPtr<VertexBuffer> >& buffers, const PODVector<unsigned>& morphRangeStarts,
        const PODVector<unsigned>& morphRangeCounts);
    /// Set index buffers.
    bool SetIndexBuffers(const Vector<SharedPtr<IndexBuffer> >& buffers);
    /// Set number of geometries.
    void SetNumGeometries(unsigned num);
    /// Set number of LOD levels in a geometry.
    bool SetNumGeometryLodLevels(unsigned index, unsigned num);
    /// Set geometry.
    bool SetGeometry(unsigned index, unsigned lodLevel, Geometry* geometry);
    /// Set geometry center.
    bool SetGeometryCenter(unsigned index, const Vector3& center);
    /// Set skeleton.
    void SetSkeleton(const Skeleton& skeleton);
    /// Set bone mappings when model has more bones than the skinning shader can handle.
    void SetGeometryBoneMappings(const Vector<PODVector<unsigned> >& geometryBoneMappings);
    /// Set vertex morphs.
    void SetMorphs(const Vector<ModelMorph>& morphs);
    /// Clone the model. The geometry data is deep-copied and can be modified in the clone without affecting the original.
    SharedPtr<Model> Clone(const String& cloneName = String::EMPTY) const;

    /// Return bounding box.
    const BoundingBox& GetBoundingBox() const { return boundingBox_; }

    /// Return skeleton.
    Skeleton& GetSkeleton() { return skeleton_; }

    /// Return vertex buffers.
    const Vector<SharedPtr<VertexBuffer> >& GetVertexBuffers() const { return vertexBuffers_; }

    /// Return index buffers.
    const Vector<SharedPtr<IndexBuffer> >& GetIndexBuffers() const { return indexBuffers_; }

    /// Return number of geometries.
    unsigned GetNumGeometries() const { return geometries_.Size(); }

    /// Return number of LOD levels in geometry.
    unsigned GetNumGeometryLodLevels(unsigned index) const;

    /// Return geometry pointers.
    const Vector<Vector<SharedPtr<Geometry> > >& GetGeometries() const { return geometries_; }

    /// Return geometry center points.
    const PODVector<Vector3>& GetGeometryCenters() const { return geometryCenters_; }

    /// Return geometry by index and LOD level. The LOD level is clamped if out of range.
    Geometry* GetGeometry(unsigned index, unsigned lodLevel) const;

    /// Return geometry center by index.
    const Vector3& GetGeometryCenter(unsigned index) const
    {
        return index < geometryCenters_.Size() ? geometryCenters_[index] : Vector3::ZERO;
    }

    /// Return geometery bone mappings.
    const Vector<PODVector<unsigned> >& GetGeometryBoneMappings() const { return geometryBoneMappings_; }

    /// Return vertex morphs.
    const Vector<ModelMorph>& GetMorphs() const { return morphs_; }

    /// Return number of vertex morphs.
    unsigned GetNumMorphs() const { return morphs_.Size(); }

    /// Return vertex morph by index.
    const ModelMorph* GetMorph(unsigned index) const;
    /// Return vertex morph by name.
    const ModelMorph* GetMorph(const String& name) const;
    /// Return vertex morph by name hash.
    const ModelMorph* GetMorph(StringHash nameHash) const;
    /// Return vertex buffer morph range start.
    unsigned GetMorphRangeStart(unsigned bufferIndex) const;
    /// Return vertex buffer morph range vertex count.
    unsigned GetMorphRangeCount(unsigned bufferIndex) const;

private:
    /// Bounding box.
    BoundingBox boundingBox_; // 包围盒
    /// Skeleton.
    Skeleton skeleton_; // 骨骼数据
    /// Vertex buffers.
    Vector<SharedPtr<VertexBuffer> > vertexBuffers_; // 顶点数据，设备缓存，对应模型的每个部分
    /// Index buffers.
    Vector<SharedPtr<IndexBuffer> > indexBuffers_; // 顶点索引数据，设备缓存，对应模型的每个部分
    /// Geometries.
    Vector<Vector<SharedPtr<Geometry> > > geometries_; // 每个模型有多个部分（换装），每个部分有多个模型精度（LOD），每个LOD用几何体（Geometry）表示，geometries_.Size()表示模型有几部分，geometries_[].Size()表示该部分的Lod层级数
    /// Geometry bone mappings.
    Vector<PODVector<unsigned> > geometryBoneMappings_; // 骨头映射数据，每个模型部分有一组
    /// Geometry centers.
    PODVector<Vector3> geometryCenters_; // 几何体的中心点
    /// Vertex morphs.
    Vector<ModelMorph> morphs_; // 顶点变形数据，每个morphs_，其buffers_[i]的变形数据都将按权重被混合到vertexBuffers_[i]中
    /// Vertex buffer morph range start.
    PODVector<unsigned> morphRangeStarts_; // vertexBuffers_中从哪个顶点开始变形
    /// Vertex buffer morph range vertex count.
    PODVector<unsigned> morphRangeCounts_; // vertexBuffers_中从morphRangeStarts_开始，有多少个顶点参与变形
    /// Vertex buffer data for asynchronous loading.
    Vector<VertexBufferDesc> loadVBData_; // 异步加载时的顶点数据在BeginLoad中存放于此地，在EndLoad中转入vertexBuffers_。
    /// Index buffer data for asynchronous loading.
    Vector<IndexBufferDesc> loadIBData_; // 异步加载时的索引数据在BeginLoad中存放于此地，在EndLoad中转入indexBuffers_。
    /// Geometry definitions for asynchronous loading.
    Vector<PODVector<GeometryDesc> > loadGeometries_; // 几何体的描述数据（模型的各个部分，每部分的各个LOD）
};

}
