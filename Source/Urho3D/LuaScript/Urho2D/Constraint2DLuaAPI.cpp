//
// Copyright (c) 2008-2016 the Urho3D project.
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

#ifdef URHO3D_URHO2D

#include "../../Precompiled.h"

#include "../../Urho2D/Constraint2D.h"
#include "../../Urho2D/RigidBody2D.h"
#include "../../LuaScript/LuaScriptUtils.h"

#include <kaguya.hpp>

namespace Urho3D
{

void RegisterConstraint2D(kaguya::State& lua)
{
    using namespace kaguya;

    lua["Constraint2D"].setClass(UserdataMetatable<Constraint2D, Component>()
        .addStaticFunction("new", &CreateObject<Constraint2D>)

        .addFunction("SetOtherBody", &Constraint2D::SetOtherBody)
        .addFunction("SetCollideConnected", &Constraint2D::SetCollideConnected)
        
        .addFunction("GetOwnerBody", &Constraint2D::GetOwnerBody)
        .addFunction("GetOtherBody", &Constraint2D::GetOtherBody)
        .addFunction("GetCollideConnected", &Constraint2D::GetCollideConnected)
        
        .addProperty("ownerBody", &Constraint2D::GetOwnerBody)
        .addProperty("otherBody", &Constraint2D::GetOtherBody, &Constraint2D::SetOtherBody)
        .addProperty("collideConnected", &Constraint2D::GetCollideConnected, &Constraint2D::SetCollideConnected)
    );
}
}

#endif