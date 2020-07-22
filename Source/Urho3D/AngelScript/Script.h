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

#include "../Core/Mutex.h"
#include "../Core/Object.h"

class asIScriptContext;
class asIScriptEngine;
class asIScriptModule;
class asITypeInfo;

struct asSMessageInfo;

namespace Urho3D
{

extern const char* LOGIC_CATEGORY;

class ResourceRouter;
class Scene;
class ScriptFile;
class ScriptInstance;

/// Output mode for DumpAPI method.
enum DumpMode
{
    DOXYGEN = 0,
    C_HEADER,
    MAX_DUMP_MODES
};

/// Scripting subsystem. Allows execution of AngelScript.
class URHO3D_API Script : public Object
{
    URHO3D_OBJECT(Script, Object);

    friend class ScriptFile;

public:
    /// Construct.
    explicit Script(Context* context);
    /// Destruct. Release the AngelScript engine.
    ~Script() override;

    /// Compile and execute a line of script in immediate mode.
    bool Execute(const String& line);
    /// Set immediate mode script file.
    void SetDefaultScriptFile(ScriptFile* file);
    /// Set immediate mode scene.
    void SetDefaultScene(Scene* scene);
    /// Set whether to execute engine console commands as script code.
    void SetExecuteConsoleCommands(bool enable);
    /// Print the whole script API (all registered classes, methods and properties) to the log. No-ops when URHO3D_LOGGING not defined.
    void DumpAPI(DumpMode mode = DOXYGEN, const String& sourceTree = String::EMPTY);
    /// Log a message from the script engine.
    void MessageCallback(const asSMessageInfo* msg);
    /// Handle a script exception.
    void ExceptionCallback(asIScriptContext* context);
    /// Get call stack.
    static String GetCallStack(asIScriptContext* context);

    /// Return the AngelScript engine.
    asIScriptEngine* GetScriptEngine() const { return scriptEngine_; }

    /// Return immediate execution script context.
    asIScriptContext* GetImmediateContext() const { return immediateContext_; }

    /// Return immediate mode script file.
    ScriptFile* GetDefaultScriptFile() const;
    /// Return immediate mode scene.
    Scene* GetDefaultScene() const;

    /// Return whether is executing engine console commands as script code.
    bool GetExecuteConsoleCommands() const { return executeConsoleCommands_; }

    /// Clear the inbuild object type cache.
    void ClearObjectTypeCache();
    /// Query for an inbuilt object type by constant declaration. Can not be used for script types.
    asITypeInfo* GetObjectType(const char* declaration);

    /// Return the script module create/delete mutex.
    Mutex& GetModuleMutex() { return moduleMutex_; }

    /// Returns an array of strings of enum value names for Enum Attributes.
    const char** GetEnumValues(int asTypeID);


private:
    /// Increase script nesting level.
    void IncScriptNestingLevel() { ++scriptNestingLevel_; }

    /// Decrease script nesting level.
    void DecScriptNestingLevel() { --scriptNestingLevel_; }

    /// Return current script nesting level.
    unsigned GetScriptNestingLevel() { return scriptNestingLevel_; }

    /// Return a script function/method execution context for the current execution nesting level.
    asIScriptContext* GetScriptFileContext();
    /// Output a sanitated row of script API. No-ops when URHO3D_LOGGING not defined.
    void OutputAPIRow(DumpMode mode, const String& row, bool removeReference = false, const String& separator = ";");
    /// Handle a console command event.
    void HandleConsoleCommand(StringHash eventType, VariantMap& eventData);

    /// AngelScript engine.
    asIScriptEngine* scriptEngine_;
    /// Immediate execution script context.
    asIScriptContext* immediateContext_;
    /// Immediate execution script file.
    WeakPtr<ScriptFile> defaultScriptFile_;
    /// Immediate execution scene.
    WeakPtr<Scene> defaultScene_;
    /// Script function/method execution contexts.
    Vector<asIScriptContext*> scriptFileContexts_;
    /// Search cache for inbuilt object types.
    HashMap<const char*, asITypeInfo*> objectTypes_;
    /// Cache of typeIds to array of enum value strings for attributes.
    HashMap<int, PODVector<const char*>> enumValues_;
    /// AngelScript resource router.
    SharedPtr<ResourceRouter> router_;
    /// Script module create/delete mutex.
    Mutex moduleMutex_;
    /// Current script execution nesting level.
    unsigned scriptNestingLevel_;
    /// Flag for executing engine console commands as script code. Default to true.
    bool executeConsoleCommands_;
};

/// Register Script library objects.
void URHO3D_API RegisterScriptLibrary(Context* context);

}

// http://www.angelcode.com/angelscript/sdk/docs/manual/doc_as_vs_cpp_types.html
// 1，对象句柄（Object handles）：
//      当AngelScript将一个对象句柄按值传递给函数（应用程序）时，它会增加参数实例的引用计数，因此该函数负责在句柄使用后释放引用。同样，AngelScript期望从函数（应用程序）返回的任何句柄都已经增加了引用（因为脚本在结束使用返回的句柄后会释放其引用计数）。
//      应用程序注册函数时使用+修饰符可以自动管理引用计数（例如，Registered as "obj@+ StoreObject(obj@+)"）。在释放参数的引用计数之前，会增加返回值的引用计数，因此让函数返回参数是可行的。
// 2，参数引用（Parameter references）：
//      因为AngelScript需要始终保证指针的有效性，它并不总是将对真实对象的引用传递给函数参数。相反，它创建一个对象的副本，该对象的引用传递给函数，如果引用被标记为返回值，则在函数返回后，副本将被复制回原始对象（如果它仍然存在）。
//      正因为如此，AngelScript的参数引用大多与C++引用或指针兼容，除了地址通常不应该存储以供以后使用，因为对象可能在函数返回后被销毁。如果需要存储对象的地址，则应使用对象句柄。
// 3，asSetGlobalMemoryFunctions，注册AngelScript用于内存管理的全局内存分配和释放函数。此函数应在asCreateScriptEngine之前调用。如果不调用，AngelScript将使用标准C库中的malloc和free函数。

// 主要接口
// asIScriptEngine，通过调用asCreateScriptEngine创建。有三个主要功能：将主程序的接口注册给脚本；编译脚本（通过asIScriptModule）；执行脚本（通过asIScriptContext）。
// asIScriptModule，通过调用asIScriptEngine::GetModule创建。将脚本文件或代码段编译（或加载已经编译过的字节码）成包含函数、类、全局变量的库。
// asIScriptContext，通过调用asCScriptEngine::CreateContext创建。为单个脚本执行提供接口。存储调用堆栈，其中保存执行所使用的局部变量，但是它不保留全局变量的副本，因为这些变量存储在asicriptModule中，asIScriptContext只做引用。
//      全局变量的值在所有上下文之间共享，因为它们都引用同一个内存。这意味着全局变量的生存期比脚本的执行时间长，并且将在执行之间保留它们的值。
//      很少有必要一次维护多个脚本上下文，唯一的例外是当一个脚本调用一个应用程序函数，而该应用程序函数又在返回之前调用另一个脚本，以及如果多个脚本并行运行。
//      尽量避免将唯一的上下文与可能需要调用脚本的每个对象相关联。相反，保留一个共享的上下文池，这些上下文可以由对象根据需要请求。
// 次要接口
// asIScriptGeneric，通用调用集。
// asIScriptObject，脚本对象的实例。所有脚本的类（class）和接口（interface）都被应用程序视为asIScriptObject类型。asIScriptObject有确定是类还是接口的方法以及与实际对象实例交互的方法。
// asITypeInfo，脚本对象的类型信息。可以表示object types, funcdefs, typedefs, and enums。
// asIScriptFunction，脚本函数描述。
// asIStringFactory，用于管理脚本使用的字符串常量。
