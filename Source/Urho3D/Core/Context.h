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
#include "../Core/Attribute.h"
#include "../Core/Object.h"

namespace Urho3D
{

/// Tracking structure for event receivers.
class URHO3D_API EventReceiverGroup : public RefCounted
{
public:
    /// Construct.
    EventReceiverGroup() :
        inSend_(0),
        dirty_(false)
    {
    }

    /// Begin event send. When receivers are removed during send, group has to be cleaned up afterward.
    void BeginSendEvent();

    /// End event send. Clean up if necessary.
    void EndSendEvent();

    /// Add receiver. Same receiver must not be double-added!
    void Add(Object* object);

    /// Remove receiver. Leave holes during send, which requires later cleanup.
    void Remove(Object* object);

    /// Receivers. May contain holes during sending.
    PODVector<Object*> receivers_;

private:
    /// "In send" recursion counter.
    unsigned inSend_;
    /// Cleanup required flag.
    bool dirty_;
};

// 执行上下文。提供对子系统、对象工厂和属性以及事件接收器的访问。
/// Urho3D execution context. Provides access to subsystems, object factories and attributes, and event receivers.
class URHO3D_API Context : public RefCounted
{
    friend class Object;

public:
    /// Construct.
    Context();
    /// Destruct.
    ~Context() override;

    /// Create an object by type. Return pointer to it or null if no factory found.
    template <class T> inline SharedPtr<T> CreateObject()
    {
        return StaticCast<T>(CreateObject(T::GetTypeStatic()));
    }
    /// Create an object by type hash. Return pointer to it or null if no factory found.
    SharedPtr<Object> CreateObject(StringHash objectType);
    /// Register a factory for an object type.
    void RegisterFactory(ObjectFactory* factory);
    /// Register a factory for an object type and specify the object category.
    void RegisterFactory(ObjectFactory* factory, const char* category);
    /// Register a subsystem.
    void RegisterSubsystem(Object* object);
    /// Remove a subsystem.
    void RemoveSubsystem(StringHash objectType);
    /// Register object attribute.
    AttributeHandle RegisterAttribute(StringHash objectType, const AttributeInfo& attr);
    /// Remove object attribute.
    void RemoveAttribute(StringHash objectType, const char* name);
    /// Remove all object attributes.
    void RemoveAllAttributes(StringHash objectType);
    /// Update object attribute's default value.
    void UpdateAttributeDefaultValue(StringHash objectType, const char* name, const Variant& defaultValue);
    /// Return a preallocated map for event data. Used for optimization to avoid constant re-allocation of event data maps.
    VariantMap& GetEventDataMap();
    /// Initialises the specified SDL systems, if not already. Returns true if successful. This call must be matched with ReleaseSDL() when SDL functions are no longer required, even if this call fails.
    bool RequireSDL(unsigned int sdlFlags);
    /// Indicate that you are done with using SDL. Must be called after using RequireSDL().
    void ReleaseSDL();
#ifdef URHO3D_IK
    /// Initialises the IK library, if not already. This call must be matched with ReleaseIK() when the IK library is no longer required.
    void RequireIK();
    /// Indicate that you are done with using the IK library.
    void ReleaseIK();
#endif

    /// Copy base class attributes to derived class.
    void CopyBaseAttributes(StringHash baseType, StringHash derivedType);
    /// Template version of registering an object factory.
    template <class T> void RegisterFactory();
    /// Template version of registering an object factory with category.
    template <class T> void RegisterFactory(const char* category);
    /// Template version of registering subsystem.
    template <class T> T* RegisterSubsystem();
    /// Template version of removing a subsystem.
    template <class T> void RemoveSubsystem();
    /// Template version of registering an object attribute.
    template <class T> AttributeHandle RegisterAttribute(const AttributeInfo& attr);
    /// Template version of removing an object attribute.
    template <class T> void RemoveAttribute(const char* name);
    /// Template version of removing all object attributes.
    template <class T> void RemoveAllAttributes();
    /// Template version of copying base class attributes to derived class.
    template <class T, class U> void CopyBaseAttributes();
    /// Template version of updating an object attribute's default value.
    template <class T> void UpdateAttributeDefaultValue(const char* name, const Variant& defaultValue);

    /// Return subsystem by type.
    Object* GetSubsystem(StringHash type) const;

    /// Return global variable based on key
    const Variant& GetGlobalVar(StringHash key) const;

    /// Return all global variables.
    const VariantMap& GetGlobalVars() const { return globalVars_; }

    /// Set global variable with the respective key and value
    void SetGlobalVar(StringHash key, const Variant& value);

    /// Return all subsystems.
    const HashMap<StringHash, SharedPtr<Object> >& GetSubsystems() const { return subsystems_; }

    /// Return all object factories.
    const HashMap<StringHash, SharedPtr<ObjectFactory> >& GetObjectFactories() const { return factories_; }

    /// Return all object categories.
    const HashMap<String, Vector<StringHash> >& GetObjectCategories() const { return objectCategories_; }

    /// Return active event sender. Null outside event handling.
    Object* GetEventSender() const;

    /// Return active event handler. Set by Object. Null outside event handling.
    EventHandler* GetEventHandler() const { return eventHandler_; }

    /// Return object type name from hash, or empty if unknown.
    const String& GetTypeName(StringHash objectType) const;
    /// Return a specific attribute description for an object, or null if not found.
    AttributeInfo* GetAttribute(StringHash objectType, const char* name);
    /// Template version of returning a subsystem.
    template <class T> T* GetSubsystem() const;
    /// Template version of returning a specific attribute description.
    template <class T> AttributeInfo* GetAttribute(const char* name);

    /// Return attribute descriptions for an object type, or null if none defined.
    const Vector<AttributeInfo>* GetAttributes(StringHash type) const
    {
        HashMap<StringHash, Vector<AttributeInfo> >::ConstIterator i = attributes_.Find(type);
        return i != attributes_.End() ? &i->second_ : nullptr;
    }

    /// Return network replication attribute descriptions for an object type, or null if none defined.
    const Vector<AttributeInfo>* GetNetworkAttributes(StringHash type) const
    {
        HashMap<StringHash, Vector<AttributeInfo> >::ConstIterator i = networkAttributes_.Find(type);
        return i != networkAttributes_.End() ? &i->second_ : nullptr;
    }

    /// Return all registered attributes.
    const HashMap<StringHash, Vector<AttributeInfo> >& GetAllAttributes() const { return attributes_; }

    /// Return event receivers for a sender and event type, or null if they do not exist.
    EventReceiverGroup* GetEventReceivers(Object* sender, StringHash eventType)
    {
        HashMap<Object*, HashMap<StringHash, SharedPtr<EventReceiverGroup> > >::Iterator i = specificEventReceivers_.Find(sender);
        if (i != specificEventReceivers_.End())
        {
            HashMap<StringHash, SharedPtr<EventReceiverGroup> >::Iterator j = i->second_.Find(eventType);
            return j != i->second_.End() ? j->second_ : nullptr;
        }
        else
            return nullptr;
    }

    /// Return event receivers for an event type, or null if they do not exist.
    EventReceiverGroup* GetEventReceivers(StringHash eventType)
    {
        HashMap<StringHash, SharedPtr<EventReceiverGroup> >::Iterator i = eventReceivers_.Find(eventType);
        return i != eventReceivers_.End() ? i->second_ : nullptr;
    }

private:
    /// Add event receiver.
    void AddEventReceiver(Object* receiver, StringHash eventType);
    /// Add event receiver for specific event.
    void AddEventReceiver(Object* receiver, Object* sender, StringHash eventType);
    /// Remove an event sender from all receivers. Called on its destruction.
    void RemoveEventSender(Object* sender);
    /// Remove event receiver from specific events.
    void RemoveEventReceiver(Object* receiver, Object* sender, StringHash eventType);
    /// Remove event receiver from non-specific events.
    void RemoveEventReceiver(Object* receiver, StringHash eventType);
    /// Begin event send.
    void BeginSendEvent(Object* sender, StringHash eventType);
    /// End event send. Clean up event receivers removed in the meanwhile.
    void EndSendEvent();

    /// Set current event handler. Called by Object.
    void SetEventHandler(EventHandler* handler) { eventHandler_ = handler; }

    // 对象工厂（建立类的反射信息，用于根据字符串创建对象）。
    // 引擎启动时，会按层次将所有ObjectFactory注册到Context，之后通过context_->CreateObject(type)创建新Object（其中type参数是类型名的StringHash）。由于factory有指向Context的指针，这样能保证每个新建对象一上来就获取到Context（Urho3D的Object都有context_成员）
    /// Object factories.
    HashMap<StringHash, SharedPtr<ObjectFactory> > factories_;
    // 各种功能单例模块，相当于某些引擎的XXXManager。通过context_->RegisterSubsystem<XXX>()创建，context_->GetSubsystem<XXX>()获取
    /// Subsystems.
    HashMap<StringHash, SharedPtr<Object> > subsystems_;
    // Object属性描述，从Serializable派生的类都能自动序列化，场景的保存/加载、网络复制都是通过Node/Component类实现，通过AttributeInfo::accessor_访问属性
    /// Attribute descriptions per object type.
    HashMap<StringHash, Vector<AttributeInfo> > attributes_;
    /// Network replication attribute descriptions per object type.
    HashMap<StringHash, Vector<AttributeInfo> > networkAttributes_;
    // 事件通信是Urho3D架构核心思想之一，而Context是事件监听的容器
    /// Event receivers for non-specific events.
    HashMap<StringHash, SharedPtr<EventReceiverGroup> > eventReceivers_; // 不限定sender的事件接收者集合
    /// Event receivers for specific senders' events.
    HashMap<Object*, HashMap<StringHash, SharedPtr<EventReceiverGroup> > > specificEventReceivers_; // 指定sender的事件接收者集合
    /// Event sender stack.
    PODVector<Object*> eventSenders_; // Object::SendEvent调用开始时，压入sender，结束时，弹出sender，用于eventDataMaps_分配
    /// Event data stack.
    PODVector<VariantMap*> eventDataMaps_; // 出于性能原因，事件参数容器是重用的，不必每次分配。
    /// Active event handler. Not stored in a stack for performance reasons; is needed only in esoteric cases.
    EventHandler* eventHandler_; // 执行中的事件处理器
    // 对象分类（例如编辑器中组件的各种类别：Audio、Geometry、Logic等）
    /// Object categories.
    HashMap<String, Vector<StringHash> > objectCategories_;
    /// Variant map for global variables that can persist throughout application execution.
    VariantMap globalVars_;
};

template <class T> void Context::RegisterFactory() { RegisterFactory(new ObjectFactoryImpl<T>(this)); }

template <class T> void Context::RegisterFactory(const char* category)
{
    RegisterFactory(new ObjectFactoryImpl<T>(this), category);
}

template <class T> T* Context::RegisterSubsystem()
{
    auto* subsystem = new T(this);
    RegisterSubsystem(subsystem);
    return subsystem;
}

template <class T> void Context::RemoveSubsystem() { RemoveSubsystem(T::GetTypeStatic()); }

template <class T> AttributeHandle Context::RegisterAttribute(const AttributeInfo& attr) { return RegisterAttribute(T::GetTypeStatic(), attr); }

template <class T> void Context::RemoveAttribute(const char* name) { RemoveAttribute(T::GetTypeStatic(), name); }

template <class T> void Context::RemoveAllAttributes() { RemoveAllAttributes(T::GetTypeStatic()); }

template <class T, class U> void Context::CopyBaseAttributes() { CopyBaseAttributes(T::GetTypeStatic(), U::GetTypeStatic()); }

template <class T> T* Context::GetSubsystem() const { return static_cast<T*>(GetSubsystem(T::GetTypeStatic())); }

template <class T> AttributeInfo* Context::GetAttribute(const char* name) { return GetAttribute(T::GetTypeStatic(), name); }

template <class T> void Context::UpdateAttributeDefaultValue(const char* name, const Variant& defaultValue)
{
    UpdateAttributeDefaultValue(T::GetTypeStatic(), name, defaultValue);
}

}
