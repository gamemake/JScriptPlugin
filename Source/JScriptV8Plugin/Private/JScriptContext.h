#pragma once

class FJScriptContext : public IJScriptContext
{
	friend class JScriptEngine;

protected:
	FJScriptContext(v8::Local<v8::Context>& Context);
	virtual ~FJScriptContext();

public:
	virtual void Expose(UClass* Class) override;
	virtual void Expose(const FString& Name, UObject* Object) override;
	virtual void Expose(const FString& Name, const FString& Script) override;
	virtual bool Execute(const FString& Script) override;

	virtual IJScriptClass* CompileScript(const FString& Filename, const FString& SourceCode) override;
	virtual void FreeScript(IJScriptClass* Class) override;

	virtual IJScriptObject* CreateObject(IJScriptClass* Class) override;
	virtual void FreeObject(IJScriptObject* Object) override;

protected:
	struct FV8Module
	{
		FString Name;
		v8::Persistent<v8::Object> Exports;
	};
	struct FV8UClass
	{
		UClass* Class;
		v8::Persistent<v8::FunctionTemplate> FunctionTemplate;
		v8::Persistent<v8::ObjectTemplate> ObjectTemplate;
		v8::Persistent<v8::Function> Function;
	};

	v8::Local<v8::Object> ConvertValue(UObject* Object);
	v8::Local<v8::Object> ConvertValue(const uint8* Data, UScriptStruct* Struct);
	v8::Local<v8::Value> ConvertValue(const uint8* Data, UProperty* Property);

	bool ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UProperty* Property);

	UClass* GetUClassByName(const FString& ClassName);
	FV8UClass* GetUClassInfo(UClass* Class);
	v8::Local<v8::FunctionTemplate> GetUClass_FunctionTemplate(UClass* Class);
	v8::Local<v8::ObjectTemplate> GetUClass_ObjectTemplate(UClass* Class);
	v8::Local<v8::Function> GetUClass_Function(UClass* Class);

private:
	static void ModuleDefine(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UClass_SetProperty(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UClass_GetProperty(const v8::FunctionCallbackInfo<v8::Value>& args);
	void UClass_Invoke(UObject* This, UFunction* Function, const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UClass_MemberInvoke(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UClass_StaticInvoke(const v8::FunctionCallbackInfo<v8::Value>& args);

	v8::Persistent<v8::Context> Context;
	TMap<FString, FV8UClass*> ClassMap;
	TMap<FString, FV8Module*> Modules;
};
