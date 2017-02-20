#include "V8PluginPCH.h"
#include "JScriptEngine.h"

#include <v8.h>
#include <libplatform/libplatform.h>
#include "JScriptContext.h"
#include "JScriptObject.h"

FJScriptObject::FJScriptObject(v8::Isolate* isloate, v8::Local<v8::Value> Object)
{
	JSObject.Reset(isolate, Object);
}

FJScriptObject::~FJScriptObject()
{
    JSObject.Reset();
}

void FJScriptObject::Invoke(const FString& Name)
{
}

void FJScriptObject::SetProperty(const FString& Name, const FString& Value)
{

}

void FJScriptObject::GetProperty(const FString& Name, FString& Value)
{

}
