#pragma once

class IJScriptClass;
class IJScriptObject;

struct FJScriptFieldInfo
{
	bool IsStatic;
	FString Name;
	bool IsArray;
	FString Type;
	UClass* PropertyClass;
	IJScriptClass* JSClass;
};

struct FJScriptMethodInfo
{
	bool IsStatic;
	FString Name;
	TArray<FJScriptFieldInfo> Params;
	FJScriptFieldInfo ReturnValue;
};

struct FJScriptClassInfo
{
	FString ClassName;
	FString ParentClassName;
	UClass* ParentClassUE;
	IJScriptClass* ParentClassJS;
	TArray<FJScriptFieldInfo> Fields;
	TArray<FJScriptMethodInfo> Methods;
};

class IJScriptClass
{
protected:
	IJScriptClass() {}
	virtual ~IJScriptClass() {}

public:
	virtual const FString& GetClassName() const = 0;
	virtual const FString& GetParentClassName() const = 0;

	virtual const TArray<FJScriptFieldInfo>& GetFields() const = 0;
	virtual const TArray<FJScriptMethodInfo>& GetMethods() const = 0;
};

class IJScriptObject
{
protected:
	IJScriptObject() {}
	virtual ~IJScriptObject() {}

public:
	virtual void Invoke(const FString& Name) = 0;
	virtual void SetProperty(const FString& Name, const FString& Value) = 0;
	virtual void GetProperty(const FString& Name, FString& Value) = 0;
};

class IJScriptContext
{
protected:
	IJScriptContext() {}
	virtual ~IJScriptContext() {}

public:
	virtual void Expose(UClass* Class) = 0;
	virtual void Expose(const FString& Name, UObject* Object) = 0;
	virtual void Expose(const FString& Name, const FString& Script) = 0;
	virtual bool Execute(const FString& Script) = 0;

	virtual IJScriptClass* CompileScript(const FString& Filename) = 0;
	virtual IJScriptClass* CompileScript(const FString& Filename, const FString& SourceCode) = 0;
	virtual void FreeScript(IJScriptClass* Class) = 0;

	virtual IJScriptClass* GetJSClass(const FString& ClassName) = 0;

	virtual IJScriptObject* CreateObject(IJScriptClass* Class) = 0;
	virtual void FreeObject(IJScriptObject* Object) = 0;
};

class JSCRIPTV8PLUGIN_API JScriptEngine
{
public:
	static void Initialize();
	static void Finalize();

	static IJScriptContext* CreateContext();
	static void FreeContext(IJScriptContext* Context);
};
