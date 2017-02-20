#include "V8PluginPCH.h"
#include "JScriptEngine.h"

#include "JScriptContext.h"
#include "JScriptClass.h"
#include "JScriptObject.h"
#include "JScriptNative.h"
#include "JScriptUtils.h"
#include "JScriptFiles.h"

static v8::Platform* v8_platform = nullptr;
static v8::Isolate::CreateParams* create_params = nullptr;
static v8::Isolate* isolate = nullptr;
static v8::Persistent<v8::Context> default_context;

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
	virtual void* Allocate(size_t length) {
		void* data = AllocateUninitialized(length);
		return data == NULL ? data : memset(data, 0, length);
	}
	virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
	virtual void Free(void* data, size_t) { free(data); }
};

void JScriptEngine::Initialize()
{
    v8::V8::InitializeICU();
	v8::V8::InitializeExternalStartupData(__argv[0]);
    v8::Platform* platform = v8::platform::CreateDefaultPlatform();
	v8::V8::InitializePlatform(platform);
	v8::V8::Initialize();
    
	create_params = new v8::Isolate::CreateParams;
	create_params->array_buffer_allocator = new ArrayBufferAllocator();
    isolate = v8::Isolate::New(*create_params);
}

void JScriptEngine::Finalize()
{
	isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::ShutdownPlatform();
	delete v8_platform;
	delete create_params->array_buffer_allocator;
	delete create_params;
}

IJScriptContext* JScriptEngine::CreateContext()
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);

	v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
	global->Set(
		v8::String::NewFromUtf8(isolate, "native_print", v8::NewStringType::kNormal).ToLocalChecked(),
		v8::FunctionTemplate::New(isolate, native_print)
	);
	global->Set(
		v8::String::NewFromUtf8(isolate, "native_abort", v8::NewStringType::kNormal).ToLocalChecked(),
		v8::FunctionTemplate::New(isolate, native_abort)
	);
	global->Set(
		v8::String::NewFromUtf8(isolate, "native_trace", v8::NewStringType::kNormal).ToLocalChecked(),
		v8::FunctionTemplate::New(isolate, native_trace)
	);
	global->Set(
		v8::String::NewFromUtf8(isolate, "native_profile", v8::NewStringType::kNormal).ToLocalChecked(),
		v8::FunctionTemplate::New(isolate, native_profile)
	);

	v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
	{
		v8::Context::Scope context_scope(context);
		v8::Local<v8::Object> g = context->Global();
		g->Set(
			v8::String::NewFromUtf8(isolate, "console", v8::NewStringType::kNormal).ToLocalChecked(),
			execute_string(isolate, context, script_console)
		);
		g->Set(
			v8::String::NewFromUtf8(isolate, "Reflect", v8::NewStringType::kNormal).ToLocalChecked(),
			execute_string(isolate, context, script_reflect_metadata)
		);
	}

	FJScriptContext* retval = new FJScriptContext(context);
    return retval;
}

void JScriptEngine::FreeContext(IJScriptContext* Context)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    delete Context;
}

FJScriptContext::FJScriptContext(v8::Local<v8::Context>& Context)
{
	this->Context.Reset(isolate, Context);
}

FJScriptContext::~FJScriptContext()
{
    this->Context.Reset();
}

void FJScriptContext::Expose(const FString& Name, UClass* Class)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = Context.Get(isolate);
    v8::Context::Scope context_scope(context);
}

void FJScriptContext::Expose(const FString& Name, UObject* Object)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = Context.Get(isolate);
    v8::Context::Scope context_scope(context);
}

void FJScriptContext::Expose(const FString& Name, const FString& Script)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = Context.Get(isolate);
	v8::Context::Scope context_scope(context);

	v8::Local<v8::Object> g = context->Global();
	g->Set(
		v8::String::NewFromUtf8(isolate, TCHAR_TO_UTF8(*Name), v8::NewStringType::kNormal).ToLocalChecked(),
		execute_string(isolate, context, TCHAR_TO_UTF8(*Script))
	);
}

void FJScriptContext::Execute(const FString& Script)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = Context.Get(isolate);
	v8::Context::Scope context_scope(context);
	execute_string(isolate, context, TCHAR_TO_UTF8(*Script));
}

IJScriptClass* FJScriptContext::CompileScript(const FString& Filename, const FString& SourceCode)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = Context.Get(isolate);
    v8::Context::Scope context_scope(context);
	v8::TryCatch try_catch;
    
	v8::Local<v8::String> file = v8::String::NewFromUtf8(isolate, TCHAR_TO_UTF8(*Filename), v8::NewStringType::kNormal).ToLocalChecked();
	v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, TCHAR_TO_UTF8(*SourceCode), v8::NewStringType::kNormal).ToLocalChecked();

	v8::Local<v8::Script> script = v8::Script::Compile(source, file);
    v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

	FJScriptClass* RetVal = new FJScriptClass(isolate);
    if(!RetVal->ProcessJSClass(result))
    {
        delete RetVal;
    }
    return RetVal;
}

void FJScriptContext::FreeScript(IJScriptClass* Class)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(Context.Get(isolate));

    delete Class;
}

IJScriptObject* FJScriptContext::CreateObject(IJScriptClass* Class)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(Context.Get(isolate));
	v8::Local<v8::Value> Object;
    auto RetVal = new FJScriptObject(isolate, Object);
    return RetVal;
}

void FJScriptContext::FreeObject(IJScriptObject* Object)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Context::Scope context_scope(Context.Get(isolate));

    delete Object;
}

void FJScriptContext::Invoke(IJScriptObject* Object, const FString& Name)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(Context.Get(isolate));

    Object->Invoke(Name);
}

void FJScriptContext::SetProperty(IJScriptObject* Object, const FString& Name, const FString& Value)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(Context.Get(isolate));

    Object->SetProperty(Name, Value);
}

void FJScriptContext::GetProperty(IJScriptObject* Object, const FString& Name, FString& Value)
{
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(Context.Get(isolate));

    Object->GetProperty(Name, Value);
}
