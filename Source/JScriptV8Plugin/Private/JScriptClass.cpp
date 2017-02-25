#include "JScriptV8PluginPCH.h"

#include "JScriptContext.h"
#include "JScriptClass.h"

FJScriptClass::FJScriptClass(v8::Local<v8::Function> Function)
{
	JSClass.Reset(v8::Isolate::GetCurrent(), Function);
}

FJScriptClass::~FJScriptClass()
{
	JSClass.Reset();
}

const FString& FJScriptClass::GetUEClassName()
{
	return UEClassName;
}

const FString& FJScriptClass::GetClassName()
{
	return ClassName;
}

TArray<FJScriptField>& FJScriptClass::GetFields()
{
	return Fields;
}

TArray<FJScriptMethod>& FJScriptClass::GetMethods()
{
	return Methods;
}

FJScriptField* FJScriptClass::GetField(FString& Name)
{
	/*
	TMap<FString, int>::const_iterator i = FieldMap.find(Name);
	if(i==FieldMap.end())
		return NULL;
	return &Fields[i->second];
	*/
	return nullptr;
}

FJScriptMethod* FJScriptClass::GetMethod(FString& Name)
{
	/*
	TMap<FString, int>::const_iterator i = MethodMap.find(Name);
	if(i==MethodMap.end())
		return NULL;
	return &Methods[i->second];
	*/
	return nullptr;
}
