#include "V8PluginPCH.h"

#include "JScriptUtils.h"

const char* ToCString(const v8::String::Utf8Value& value) {
	return *value ? *value : "<string conversion failed>";
}

v8::Local<v8::String> LoadFileToString(v8::Isolate* isolate, const char* filename)
{
	v8::Local<v8::String> retval;
	FString Text;
	if (FFileHelper::LoadFileToString(Text, UTF8_TO_TCHAR(filename)))
	{
		retval = v8::String::NewFromUtf8(isolate, TCHAR_TO_UTF8(*Text), v8::NewStringType::kNormal).ToLocalChecked();
	}
	return retval;
}

v8::Local<v8::Value> ExecuteString(v8::Isolate* isolate, v8::Local<v8::Context>& context, const char* string)
{
	v8::Local<v8::Value> retval;
	v8::Local<v8::String> v8string = v8::String::NewFromUtf8(isolate, string, v8::NewStringType::kNormal).ToLocalChecked();
	v8::TryCatch try_catch(isolate);
	v8::Local<v8::Script> script;
	if (!v8::Script::Compile(context, v8string).ToLocal(&script))
	{
		report_exception(isolate, try_catch);
	}
	else
	{
		if (!script->Run(context).ToLocal(&retval))
		{
			check(try_catch.HasCaught());
			report_exception(isolate, try_catch);
			retval.Clear();
		}
	}
	return retval;
}

void ReportExeception(v8::Isolate* isolate, v8::TryCatch& try_catch)
{
	v8::HandleScope handle_scope(isolate);
	v8::String::Utf8Value exception(try_catch.Exception());
	const char* exception_string = ToCString(exception);
	v8::Local<v8::Message> message = try_catch.Message();
	if (message.IsEmpty())
	{
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		fprintf(stderr, "%s\n", exception_string);
	}
	else
	{
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
		v8::Local<v8::Context> context(isolate->GetCurrentContext());
		const char* filename_string = ToCString(filename);
		int linenum = message->GetLineNumber(context).FromJust();
		fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
		// Print line of source code.
		v8::String::Utf8Value sourceline(
			message->GetSourceLine(context).ToLocalChecked());
		const char* sourceline_string = ToCString(sourceline);
		fprintf(stderr, "%s\n", sourceline_string);
		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn(context).FromJust();
		for (int i = 0; i < start; i++) {
			fprintf(stderr, " ");
		}
		int end = message->GetEndColumn(context).FromJust();
		for (int i = start; i < end; i++) {
			fprintf(stderr, "^");
		}
		fprintf(stderr, "\n");
		v8::Local<v8::Value> stack_trace_string;
		if (try_catch.StackTrace(context).ToLocal(&stack_trace_string) &&
			stack_trace_string->IsString() &&
			v8::Local<v8::String>::Cast(stack_trace_string)->Length() > 0) {
			v8::String::Utf8Value stack_trace(stack_trace_string);
			const char* msg = ToCString(stack_trace);
			fprintf(stderr, "%s\n", msg);
		}
	}
}
