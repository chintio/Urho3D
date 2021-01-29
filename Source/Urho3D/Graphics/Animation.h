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

#include "../Container/FlagSet.h"
#include "../Container/Ptr.h"
#include "../Math/Quaternion.h"
#include "../Math/Vector3.h"
#include "../Resource/Resource.h"

namespace Urho3D
{

enum AnimationChannel : unsigned char
{
    CHANNEL_NONE = 0x0,
    CHANNEL_POSITION = 0x1,
    CHANNEL_ROTATION = 0x2,
    CHANNEL_SCALE = 0x4,
};
URHO3D_FLAGSET(AnimationChannel, AnimationChannelFlags);

/// Skeletal animation keyframe.
struct AnimationKeyFrame
{
    /// Construct.
    AnimationKeyFrame() :
        time_(0.0f),
        scale_(Vector3::ONE)
    {
    }

    /// Keyframe time.
    float time_;
    /// Bone position.
    Vector3 position_;
    /// Bone rotation.
    Quaternion rotation_;
    /// Bone scale.
    Vector3 scale_;
};

/// Skeletal animation track, stores keyframes of a single bone.
struct URHO3D_API AnimationTrack
{
    /// Construct.
    AnimationTrack()
    {
    }

    /// Assign keyframe at index.
    void SetKeyFrame(unsigned index, const AnimationKeyFrame& keyFrame);
    /// Add a keyframe at the end.
    void AddKeyFrame(const AnimationKeyFrame& keyFrame);
    /// Insert a keyframe at index.
    void InsertKeyFrame(unsigned index, const AnimationKeyFrame& keyFrame);
    /// Remove a keyframe at index.
    void RemoveKeyFrame(unsigned index);
    /// Remove all keyframes.
    void RemoveAllKeyFrames();

    /// Return keyframe at index, or null if not found.
    AnimationKeyFrame* GetKeyFrame(unsigned index);
    /// Return number of keyframes.
    unsigned GetNumKeyFrames() const { return keyFrames_.Size(); }
    /// Return keyframe index based on time and previous index.
    void GetKeyFrameIndex(float time, unsigned& index) const;

    /// Bone or scene node name.
    String name_; // 轨迹名称，和骨头名称对应
    /// Name hash.
    StringHash nameHash_;
    /// Bitmask of included data (position, rotation, scale.)
    AnimationChannelFlags channelMask_{}; // 通道掩码（通过位标识，指明动画轨迹是否包含位置、旋转、缩放数据）
    /// Keyframes.
    Vector<AnimationKeyFrame> keyFrames_; // 各个关键帧数据（时间、位置、旋转、缩放）
};

/// %Animation trigger point.
struct AnimationTriggerPoint
{
    /// Construct.
    AnimationTriggerPoint() :
        time_(0.0f)
    {
    }

    /// Trigger time.
    float time_; // 触发时间点
    /// Trigger data.
    Variant data_;
};

/// Skeletal animation resource.
class URHO3D_API Animation : public ResourceWithMetadata
{
    URHO3D_OBJECT(Animation, ResourceWithMetadata);

public:
    /// Construct.
    explicit Animation(Context* context);
    /// Destruct.
    ~Animation() override;
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Load resource from stream. May be called from a worker thread. Return true if successful.
    bool BeginLoad(Deserializer& source) override;
    /// Save resource. Return true if successful.
    bool Save(Serializer& dest) const override;

    /// Set animation name.
    void SetAnimationName(const String& name);
    /// Set animation length.
    void SetLength(float length);
    /// Create and return a track by name. If track by same name already exists, returns the existing.
    AnimationTrack* CreateTrack(const String& name);
    /// Remove a track by name. Return true if was found and removed successfully. This is unsafe if the animation is currently used in playback.
    bool RemoveTrack(const String& name);
    /// Remove all tracks. This is unsafe if the animation is currently used in playback.
    void RemoveAllTracks();
    /// Set a trigger point at index.
    void SetTrigger(unsigned index, const AnimationTriggerPoint& trigger);
    /// Add a trigger point.
    void AddTrigger(const AnimationTriggerPoint& trigger);
    /// Add a trigger point.
    void AddTrigger(float time, bool timeIsNormalized, const Variant& data);
    /// Remove a trigger point by index.
    void RemoveTrigger(unsigned index);
    /// Remove all trigger points.
    void RemoveAllTriggers();
    /// Resize trigger point vector.
    void SetNumTriggers(unsigned num);
    /// Clone the animation.
    SharedPtr<Animation> Clone(const String& cloneName = String::EMPTY) const;

    /// Return animation name.
    const String& GetAnimationName() const { return animationName_; }

    /// Return animation name hash.
    StringHash GetAnimationNameHash() const { return animationNameHash_; }

    /// Return animation length.
    float GetLength() const { return length_; }

    /// Return all animation tracks.
    const HashMap<StringHash, AnimationTrack>& GetTracks() const { return tracks_; }

    /// Return number of animation tracks.
    unsigned GetNumTracks() const { return tracks_.Size(); }

    /// Return animation track by index.
    AnimationTrack *GetTrack(unsigned index);

    /// Return animation track by name.
    AnimationTrack* GetTrack(const String& name);
    /// Return animation track by name hash.
    AnimationTrack* GetTrack(StringHash nameHash);

    /// Return animation trigger points.
    const Vector<AnimationTriggerPoint>& GetTriggers() const { return triggers_; }

    /// Return number of animation trigger points.
    unsigned GetNumTriggers() const { return triggers_.Size(); }

    /// Return a trigger point by index.
    AnimationTriggerPoint* GetTrigger(unsigned index);

private:
    /// Animation name.
    String animationName_; // 动画名称
    /// Animation name hash.
    StringHash animationNameHash_;
    /// Animation length.
    float length_; // 动画长度，秒
    /// Animation tracks.
    HashMap<StringHash, AnimationTrack> tracks_; // 动画轨迹，每块骨头一个轨迹
    /// Animation trigger points.
    Vector<AnimationTriggerPoint> triggers_; // 触发器数据
};

}
