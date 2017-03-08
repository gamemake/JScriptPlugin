#pragma once

class FJScriptClass;
class FJScriptObject;

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

	virtual IJScriptClass* CompileScript(const FString& Filename) override;
	virtual IJScriptClass* CompileScript(const FString& Filename, const FString& SourceCode) override;
	virtual void FreeScript(IJScriptClass* Class) override;

	virtual IJScriptClass* GetJSClass(const FString& ClassName) override;

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
	struct FV8UStruct
	{
		UStruct* Struct;
		v8::Persistent<v8::FunctionTemplate> FunctionTemplate;
		v8::Persistent<v8::ObjectTemplate> ObjectTemplate;
		v8::Persistent<v8::Function> Function;
	};

	v8::Local<v8::Value> ConvertValue(const uint8* Data, UProperty* Property);
	v8::Local<v8::Value> ConvertValue(UObject* Object);
	v8::Local<v8::Value> ConvertValue(const uint8* Data, UClassProperty* ClassProperty);
	v8::Local<v8::Value> ConvertValue(const uint8* Data, UStructProperty* StructProperty);
	v8::Local<v8::Value> ConvertValue(const uint8* Data, UArrayProperty* ArrayProperty);
	v8::Local<v8::Value> ConvertValue(const uint8* Data, USetProperty* SetProperty);
	v8::Local<v8::Value> ConvertValue(const uint8* Data, UMapProperty* MapProperty);
	
	bool ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UProperty* Property);
	UObject* ConvertJSValue(v8::Local<v8::Value>& JSValue);
	bool ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UClassProperty* ClassProperty);
	bool ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UStructProperty* StructProperty);
	bool ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UArrayProperty* ArrayProperty);
	bool ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, USetProperty* SetProperty);
	bool ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UMapProperty* MapProperty);

	FV8UClass* GetUClassByName(const FString& ClassName);
	FV8UClass* GetUClassInfo(UClass* Class);

	FV8UStruct* GetUStructByName(const FString& StructName);
	FV8UStruct* GetUStructInfo(UStruct* Struct);

private:
	static void ModuleDefine(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void RegisterJSClass(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UClass_SetProperty(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UClass_GetProperty(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UClass_Invoke(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UStruct_SetProperty(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void UStruct_GetProperty(const v8::FunctionCallbackInfo<v8::Value>& args);

	static void TArray_Empty(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TArray_Get(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TArray_Set(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TArray_Num(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TArray_Push(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TArray_Pop(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TArray_Foreach(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TMap_Empty(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TMap_Get(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TMap_Set(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TMap_Foreach(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TSet_Empty(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TSet_Add(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TSet_Remove(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TSet_Exists(const v8::FunctionCallbackInfo<v8::Value>& args);
	static void TSet_Foreach(const v8::FunctionCallbackInfo<v8::Value>& args);


	v8::Persistent<v8::Context> Context;
	v8::Persistent<v8::Function> UpdateBPInfo;

	TMap<FString, FJScriptClass*> JSClassMap;
	TMap<FString, FV8Module*> Modules;

	TMap<FString, FV8UClass*> UClassMap;
	TMap<FString, FV8UStruct*> UStructMap;
	v8::Persistent<v8::ObjectTemplate> TArrayTemplate;
	v8::Persistent<v8::ObjectTemplate> TSetTemplate;
	v8::Persistent<v8::ObjectTemplate> TMapTemplate;
};
