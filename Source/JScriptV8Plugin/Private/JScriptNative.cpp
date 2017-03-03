#include "JScriptV8PluginPCH.h"

#include "JScriptNative.h"

void native_print(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	if (args.Length() == 2 && args[0]->IsString() && args[1]->IsString())
	{
		v8::HandleScope handle_scope(args.GetIsolate());
		v8::String::Utf8Value level(args[0]);
		v8::String::Utf8Value info(args[1]);

		if (strcmp(*level, "error") == 0)
		{
			UE_LOG(LogJScriptV8Plugin, Error, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
		else if (strcmp(*level, "warning") == 0)
		{
			UE_LOG(LogJScriptV8Plugin, Warning, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
		else if (strcmp(*level, "info") == 0)
		{
			UE_LOG(LogJScriptV8Plugin, Verbose, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
		else if (strcmp(*level, "log") == 0)
		{
			UE_LOG(LogJScriptV8Plugin, Log, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
		else
		{
			UE_LOG(LogJScriptV8Plugin, VeryVerbose, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
	}
}

void native_abort(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	if (args.Length() == 1 && args[0]->IsString())
	{
		v8::String::Utf8Value info(args[0]);
		printf("==== ASSERT : %s\n", *info);
	}
}

void native_trace(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	printf("do trace\n");
}

void native_profile(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	if (args.Length() == 1)
	{
		if (args[0]->IsString())
		{
			v8::HandleScope handle_scope(args.GetIsolate());
			v8::String::Utf8Value name(args[0]);
			printf("profile %s\n", *name);
		}
	}
	else if (args.Length() == 0)
	{
		printf("profile end\n");
	}
}

void tarray_constructor(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tarray_empty(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tarray_get(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tarray_set(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tarray_num(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tarray_push(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tarray_pop(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tarray_foreach(const v8::FunctionCallbackInfo<v8::Value>& args) {}

void tmap_constructor(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_empty(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_add(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_remove(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_set(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_find(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_foreach(const v8::FunctionCallbackInfo<v8::Value>& args) {}

void tmap_constructor(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_empty(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_add(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_remove(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_exists(const v8::FunctionCallbackInfo<v8::Value>& args) {}
void tmap_foreach(const v8::FunctionCallbackInfo<v8::Value>& args) {}
