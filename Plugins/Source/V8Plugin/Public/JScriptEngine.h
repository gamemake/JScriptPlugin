#pragma once

struct FJScriptField
{
    FString Name;
    UClass* Type;
};

struct FJScriptMethod
{
    FString Name;
    TArray<FJScriptField> Parameters;
    FJScriptField ReturnValue;
};

class FJScriptContext;
class JScriptEngine;

class IJScriptClass
{
	friend class FJScriptContext;

protected:
    virtual ~IJScriptClass() {}

public:
    virtual const FString& GetUEClassName() = 0;
    virtual const FString& GetClassName() = 0;

    virtual TArray<FJScriptField>& GetFields() = 0;
    virtual TArray<FJScriptMethod>& GetMethods() = 0;
};

class IJScriptObject
{
	friend class FJScriptContext;

protected:
    virtual ~IJScriptObject() {}

public:
    virtual void Invoke(const FString& Name) = 0;
    virtual void SetProperty(const FString& Name, const FString& Value) = 0;
    virtual void GetProperty(const FString& Name, FString& Value) = 0;
};

class IJScriptContext
{
	friend class JScriptEngine;

protected:
    virtual ~IJScriptContext() {}

public:
	virtual void Expose(const FString& Name, UClass* Class) = 0;
	virtual void Expose(const FString& Name, UObject* Object) = 0;
	virtual void Expose(const FString& Name, const FString& Script) = 0;
	virtual void Execute(const FString& Script) = 0;

    virtual IJScriptClass* CompileScript(const FString& Filename, const FString& SourceCode) = 0;
    virtual void FreeScript(IJScriptClass* Class) = 0;

    virtual IJScriptObject* CreateObject(IJScriptClass* Class) = 0;
    virtual void FreeObject(IJScriptObject* Object) = 0;

    virtual void Invoke(IJScriptObject* Object, const FString& Name) = 0;
    virtual void SetProperty(IJScriptObject* Object, const FString& Name, const FString& Value) = 0;
    virtual void GetProperty(IJScriptObject* Object, const FString& Name, FString& Value) = 0;
};

class V8PLUGIN_API JScriptEngine
{
public:
    static void Initialize();
    static void Finalize();

    static IJScriptContext* CreateContext();
    static void FreeContext(IJScriptContext* Context);
};
