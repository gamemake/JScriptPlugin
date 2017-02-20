#include "V8PluginPCH.h"
#include "JScriptEngine.h"

#include <v8.h>
#include "JScriptContext.h"
#include "JScriptClass.h"

FJScriptClass::FJScriptClass(v8::Isolate* isloate)
{
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

bool FJScriptClass::ProcessJSClass(v8::Local<v8::Value>& Class)
{
	return false;
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
