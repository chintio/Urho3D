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

#include "../Precompiled.h"

#include "../Math/Frustum.h"
#include "../Math/Polyhedron.h"

#include "../DebugNew.h"

namespace Urho3D
{

// 用顶点序列vertices的包围球定义this
void Sphere::Define(const Vector3* vertices, unsigned count)
{
    if (!count)
        return;

    Clear();
    Merge(vertices, count);
}

// 用包围盒box的包围球定义this
void Sphere::Define(const BoundingBox& box)
{
    const Vector3& min = box.min_;
    const Vector3& max = box.max_;

    Clear();
    Merge(min);
    Merge(Vector3(max.x_, min.y_, min.z_));
    Merge(Vector3(min.x_, max.y_, min.z_));
    Merge(Vector3(max.x_, max.y_, min.z_));
    Merge(Vector3(min.x_, min.y_, max.z_));
    Merge(Vector3(max.x_, min.y_, max.z_));
    Merge(Vector3(min.x_, max.y_, max.z_));
    Merge(max);
}

// 用截锥体frustum的包围球定义this
void Sphere::Define(const Frustum& frustum)
{
    Define(frustum.vertices_, NUM_FRUSTUM_VERTICES);
}

// 用多面体poly的包围球定义this
void Sphere::Define(const Polyhedron& poly)
{
    Clear();
    Merge(poly);
}

// 根据this、顶点序列vertices，创建包围球，作为新的this
void Sphere::Merge(const Vector3* vertices, unsigned count)
{
    while (count--)
        Merge(*vertices++);
}

// 根据this、box创建包围球，作为新的this
void Sphere::Merge(const BoundingBox& box)
{
    const Vector3& min = box.min_;
    const Vector3& max = box.max_;

    Merge(min);
    Merge(Vector3(max.x_, min.y_, min.z_));
    Merge(Vector3(min.x_, max.y_, min.z_));
    Merge(Vector3(max.x_, max.y_, min.z_));
    Merge(Vector3(min.x_, min.y_, max.z_));
    Merge(Vector3(max.x_, min.y_, max.z_));
    Merge(Vector3(min.x_, max.y_, max.z_));
    Merge(max);
}

// 根据this、frustum创建包围球，作为新的this
void Sphere::Merge(const Frustum& frustum)
{
    const Vector3* vertices = frustum.vertices_;
    Merge(vertices, NUM_FRUSTUM_VERTICES);
}

// 根据this、poly创建包围球，作为新的this
void Sphere::Merge(const Polyhedron& poly)
{
    for (unsigned i = 0; i < poly.faces_.Size(); ++i)
    {
        const PODVector<Vector3>& face = poly.faces_[i];
        if (!face.Empty())
            Merge(&face[0], face.Size());
    }
}

// 根据this、sphere创建包围球，作为新的this
void Sphere::Merge(const Sphere& sphere)
{
    // 使用sphere初始化this
    if (radius_ < 0.0f)
    {
        center_ = sphere.center_;
        radius_ = sphere.radius_;
        return;
    }

    Vector3 offset = sphere.center_ - center_;
    float dist = offset.Length(); // 球心距离

    // If sphere fits inside, do nothing
    if (dist + sphere.radius_ < radius_) // 如果sphere在this内部，什么也不做
        return;

    // If we fit inside the other sphere, become it
    if (dist + radius_ < sphere.radius_) // 如果this在sphere内部，则用sphere替换this
    {
        center_ = sphere.center_;
        radius_ = sphere.radius_;
    }
    else // 否则，以两球体的包围球创建新的球体
    {
        Vector3 NormalizedOffset = offset / dist;

        Vector3 min = center_ - radius_ * NormalizedOffset;
        Vector3 max = sphere.center_ + sphere.radius_ * NormalizedOffset;
        center_ = (min + max) * 0.5f;
        radius_ = (max - center_).Length();
    }
}

// 包围盒box和this的位置关系
Intersection Sphere::IsInside(const BoundingBox& box) const
{
    float radiusSquared = radius_ * radius_; // 半径平方
    float distSquared = 0; // 距离平方
    float temp;
    Vector3 min = box.min_;
    Vector3 max = box.max_;

    // 如果球心在box外部，则计算球心到box包围盒的最近距离
    if (center_.x_ < min.x_)
    {
        temp = center_.x_ - min.x_;
        distSquared += temp * temp;
    }
    else if (center_.x_ > max.x_)
    {
        temp = center_.x_ - max.x_;
        distSquared += temp * temp;
    }
    if (center_.y_ < min.y_)
    {
        temp = center_.y_ - min.y_;
        distSquared += temp * temp;
    }
    else if (center_.y_ > max.y_)
    {
        temp = center_.y_ - max.y_;
        distSquared += temp * temp;
    }
    if (center_.z_ < min.z_)
    {
        temp = center_.z_ - min.z_;
        distSquared += temp * temp;
    }
    else if (center_.z_ > max.z_)
    {
        temp = center_.z_ - max.z_;
        distSquared += temp * temp;
    }

    if (distSquared >= radiusSquared) // 球在box外部（包含外切）
        return OUTSIDE;

    min -= center_;
    max -= center_;

    Vector3 tempVec = min; // - - -
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = max.x_; // + - -
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = max.y_; // + + -
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = min.x_; // - + -
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.z_ = max.z_; // - + +
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = min.y_; // - - +
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.x_ = max.x_; // + - +
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;
    tempVec.y_ = max.y_; // + + +
    if (tempVec.LengthSquared() >= radiusSquared)
        return INTERSECTS;

    return INSIDE;
}

// 包围盒box和this的位置关系
Intersection Sphere::IsInsideFast(const BoundingBox& box) const
{
    float radiusSquared = radius_ * radius_;
    float distSquared = 0;
    float temp;
    Vector3 min = box.min_;
    Vector3 max = box.max_;

    if (center_.x_ < min.x_)
    {
        temp = center_.x_ - min.x_;
        distSquared += temp * temp;
    }
    else if (center_.x_ > max.x_)
    {
        temp = center_.x_ - max.x_;
        distSquared += temp * temp;
    }
    if (center_.y_ < min.y_)
    {
        temp = center_.y_ - min.y_;
        distSquared += temp * temp;
    }
    else if (center_.y_ > max.y_)
    {
        temp = center_.y_ - max.y_;
        distSquared += temp * temp;
    }
    if (center_.z_ < min.z_)
    {
        temp = center_.z_ - min.z_;
        distSquared += temp * temp;
    }
    else if (center_.z_ > max.z_)
    {
        temp = center_.z_ - max.z_;
        distSquared += temp * temp;
    }

    if (distSquared >= radiusSquared)
        return OUTSIDE;
    else
        return INSIDE;
}

Vector3 Sphere::GetLocalPoint(float theta, float phi) const
{
    return Vector3(
        radius_ * Sin(theta) * Sin(phi),
        radius_ * Cos(phi),
        radius_ * Cos(theta) * Sin(phi)
    );
}

}
