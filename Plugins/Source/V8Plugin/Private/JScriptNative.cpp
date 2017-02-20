#include "V8PluginPCH.h"

#include "JScriptNative.h"

void native_print(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	if (args.Length() == 2 && args[0]->IsString() && args[1]->IsString())
	{
		v8::HandleScope handle_scope(args.GetIsolate());
		v8::String::Utf8Value level(args[0]);
		v8::String::Utf8Value info(args[1]);

		if (strcmp(*level, "error"))
		{
			UE_LOG(LogV8Plugin, Error, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
		else if (strcmp(*level, "warning"))
		{
			UE_LOG(LogV8Plugin, Warning, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
		else if (strcmp(*level, "info"))
		{
			UE_LOG(LogV8Plugin, Verbose, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
		else if (strcmp(*level, "log"))
		{
			UE_LOG(LogV8Plugin, Log, TEXT("%s"), UTF8_TO_TCHAR(*info));
		}
		else
		{
			UE_LOG(LogV8Plugin, VeryVerbose, TEXT("%s"), UTF8_TO_TCHAR(*info));
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
