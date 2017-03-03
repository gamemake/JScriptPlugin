#include "JScriptV8PluginPCH.h"

#include "JScriptUtils.h"

const char* ToCString(const v8::String::Utf8Value& value)
{
	return *value ? *value : "<string conversion failed>";
}

v8::Local<v8::String> ToV8String(v8::Isolate* isolate, const char* value)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto retval = v8::String::NewFromUtf8(isolate, value, v8::NewStringType::kNormal).ToLocalChecked();
	return handle_scope.Escape(retval);
}

v8::Local<v8::String> ToV8String(v8::Isolate* isolate, const FString& value)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto retval = v8::String::NewFromUtf8(isolate, TCHAR_TO_UTF8(*value), v8::NewStringType::kNormal).ToLocalChecked();
	return handle_scope.Escape(retval);
}

v8::Local<v8::Name> ToV8Name(v8::Isolate* isolate, const char* name)
{
	v8::EscapableHandleScope handle_scope(isolate);
	v8::Local<v8::Name> retval = ToV8String(isolate, name);
	return handle_scope.Escape(retval);
}

FString ToUEString(v8::Local<v8::String> value)
{
	FString retval;
	v8::String::Utf8Value str(value);
	retval = UTF8_TO_TCHAR(*str);
	return retval;
}

FString ToUEString(v8::Local<v8::Value> value)
{
	FString retval;
	auto jsstr = v8::Local<v8::String>::Cast(value);
	if (!jsstr.IsEmpty())
	{
		v8::String::Utf8Value str(value);
		retval = UTF8_TO_TCHAR(*str);
	}
	return retval;
}

bool GetClassName(v8::Isolate* isolate, v8::Local<v8::Value>& JSValue, FString& ClassName)
{
	v8::HandleScope handle_scope(isolate);

	auto Function = v8::Local<v8::Function>::Cast(JSValue);
	if (!Function.IsEmpty())
	{
	}
	return false;
}

v8::Local<v8::String> LoadFileToString(v8::Isolate* isolate, const char* filename)
{
	v8::EscapableHandleScope handle_scope(isolate);
	v8::Local<v8::String> retval;
	FString Text;
	if (FFileHelper::LoadFileToString(Text, UTF8_TO_TCHAR(filename)))
	{
		retval = ToV8String(isolate, TCHAR_TO_UTF8(*Text));
	}
	return handle_scope.Escape(retval);
}

v8::Local<v8::Value> ExecuteString(v8::Isolate* isolate, v8::Local<v8::Context>& context, const char* string)
{
	v8::EscapableHandleScope handle_scope(isolate);
	v8::Local<v8::Value> retval;
	v8::Local<v8::String> v8string = v8::String::NewFromUtf8(isolate, string, v8::NewStringType::kNormal).ToLocalChecked();
	v8::TryCatch try_catch(isolate);
	v8::Local<v8::Script> script;
	if (!v8::Script::Compile(context, v8string).ToLocal(&script))
	{
		ReportException(isolate, try_catch);
	}
	else
	{
		if (!script->Run(context).ToLocal(&retval))
		{
			check(try_catch.HasCaught());
			ReportException(isolate, try_catch);
			retval.Clear();
		}
	}
	return handle_scope.Escape(retval);
}

void ReportException(v8::Isolate* isolate, v8::TryCatch& try_catch)
{
	v8::HandleScope handle_scope(isolate);
	v8::String::Utf8Value exception(try_catch.Exception());
	const char* exception_string = ToCString(exception);
	v8::Local<v8::Message> message = try_catch.Message();
	if (message.IsEmpty())
	{
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		UE_LOG(LogJScriptV8Plugin, Error, TEXT("%s"), UTF8_TO_TCHAR(exception_string));
	}
	else
	{
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> context(isolate->GetCurrentContext());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber(context).FromJust();
		UE_LOG(LogJScriptV8Plugin, Error, TEXT("%s:%i: %s"), UTF8_TO_TCHAR(filename_string), linenum, UTF8_TO_TCHAR(exception_string));
		// Print line of source code.
		v8::String::Utf8Value sourceline(message->GetSourceLine(context).ToLocalChecked());
		const char* sourceline_string = ToCString(sourceline);
		UE_LOG(LogJScriptV8Plugin, Error, TEXT("%s"), UTF8_TO_TCHAR(sourceline_string));
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn(context).FromJust();
		for (int i = 0; i < start; i++) {
			fprintf(stderr, " ");
		}
		int end = message->GetEndColumn(context).FromJust();
		for (int i = start; i < end; i++) {
			fprintf(stderr, "^");
		}
		UE_LOG(LogJScriptV8Plugin, Error, TEXT("%s%s"),
			TEXT(""), TEXT("")
		);

		v8::Local<v8::Value> stack_trace_string;
		if (try_catch.StackTrace(context).ToLocal(&stack_trace_string) &&
			stack_trace_string->IsString() &&
			v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
			v8::String::Utf8Value stack_trace(stack_trace_string);
			const char* msg = ToCString(stack_trace);
			UE_LOG(LogJScriptV8Plugin, Error, TEXT("%s"), UTF8_TO_TCHAR(msg));
		}
	}
}

void ThrowException(v8::Isolate* isolate, const char* Message)
{
	isolate->ThrowException(v8::Exception::Error(ToV8String(isolate, Message)));
}
