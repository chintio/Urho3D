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
// 1����������Object handles����
//      ��AngelScript��һ����������ֵ���ݸ�������Ӧ�ó���ʱ���������Ӳ���ʵ�������ü�������˸ú��������ھ��ʹ�ú��ͷ����á�ͬ����AngelScript�����Ӻ�����Ӧ�ó��򣩷��ص��κξ�����Ѿ����������ã���Ϊ�ű��ڽ���ʹ�÷��صľ������ͷ������ü�������
//      Ӧ�ó���ע�ắ��ʱʹ��+���η������Զ��������ü��������磬Registered as "obj@+ StoreObject(obj@+)"�������ͷŲ��������ü���֮ǰ�������ӷ���ֵ�����ü���������ú������ز����ǿ��еġ�
// 2���������ã�Parameter references����
//      ��ΪAngelScript��Ҫʼ�ձ�ָ֤�����Ч�ԣ����������ǽ�����ʵ��������ô��ݸ������������෴��������һ������ĸ������ö�������ô��ݸ�������������ñ����Ϊ����ֵ�����ں������غ󣬸����������ƻ�ԭʼ�����������Ȼ���ڣ���
//      ����Ϊ��ˣ�AngelScript�Ĳ������ô����C++���û�ָ����ݣ����˵�ַͨ����Ӧ�ô洢�Թ��Ժ�ʹ�ã���Ϊ��������ں������غ����١������Ҫ�洢����ĵ�ַ����Ӧʹ�ö�������
// 3��asSetGlobalMemoryFunctions��ע��AngelScript�����ڴ�����ȫ���ڴ������ͷź������˺���Ӧ��asCreateScriptEngine֮ǰ���á���������ã�AngelScript��ʹ�ñ�׼C���е�malloc��free������

// ��Ҫ�ӿ�
// asIScriptEngine��ͨ������asCreateScriptEngine��������������Ҫ���ܣ���������Ľӿ�ע����ű�������ű���ͨ��asIScriptModule����ִ�нű���ͨ��asIScriptContext����
// asIScriptModule��ͨ������asIScriptEngine::GetModule���������ű��ļ������α��루������Ѿ���������ֽ��룩�ɰ����������ࡢȫ�ֱ����Ŀ⡣
// asIScriptContext��ͨ������asCScriptEngine::CreateContext������Ϊ�����ű�ִ���ṩ�ӿڡ��洢���ö�ջ�����б���ִ����ʹ�õľֲ�������������������ȫ�ֱ����ĸ�������Ϊ��Щ�����洢��asicriptModule�У�asIScriptContextֻ�����á�
//      ȫ�ֱ�����ֵ������������֮�乲����Ϊ���Ƕ�����ͬһ���ڴ档����ζ��ȫ�ֱ����������ڱȽű���ִ��ʱ�䳤�����ҽ���ִ��֮�䱣�����ǵ�ֵ��
//      �����б�Ҫһ��ά������ű������ģ�Ψһ�������ǵ�һ���ű�����һ��Ӧ�ó�����������Ӧ�ó��������ڷ���֮ǰ������һ���ű����Լ��������ű��������С�
//      �������⽫Ψһ���������������Ҫ���ýű���ÿ��������������෴������һ������������ĳأ���Щ�����Ŀ����ɶ��������Ҫ����
// ��Ҫ�ӿ�
// asIScriptGeneric��ͨ�õ��ü���
// asIScriptObject���ű������ʵ�������нű����ࣨclass���ͽӿڣ�interface������Ӧ�ó�����ΪasIScriptObject���͡�asIScriptObject��ȷ�����໹�ǽӿڵķ����Լ���ʵ�ʶ���ʵ�������ķ�����
// asITypeInfo���ű������������Ϣ�����Ա�ʾobject types, funcdefs, typedefs, and enums��
// asIScriptFunction���ű�����������
// asIStringFactory�����ڹ���ű�ʹ�õ��ַ���������
