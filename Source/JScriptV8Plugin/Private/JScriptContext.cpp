#include "JScriptV8PluginPCH.h"

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

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
{
public:
	virtual void* Allocate(size_t length)
	{
		void* data = AllocateUninitialized(length);
		return data == NULL ? data : memset(data, 0, length);
	}
	virtual void* AllocateUninitialized(size_t length)
	{
		return malloc(length);
	}
	virtual void Free(void* data, size_t)
	{
		free(data);
	}
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
		ToV8Name(isolate, "native_print"),
		v8::FunctionTemplate::New(isolate, native_print)
	);
	global->Set(
		ToV8Name(isolate, "native_abort"),
		v8::FunctionTemplate::New(isolate, native_abort)
	);
	global->Set(
		ToV8Name(isolate, "native_trace"),
		v8::FunctionTemplate::New(isolate, native_trace)
	);
	global->Set(
		ToV8Name(isolate, "native_profile"),
		v8::FunctionTemplate::New(isolate, native_profile)
	);

	v8::Local<v8::Context> context = v8::Context::New(isolate, NULL, global);
	FJScriptContext* retval = new FJScriptContext(context);

	{
		v8::Context::Scope context_scope(context);
		v8::Local<v8::Object> g = context->Global();
		g->Set(
			ToV8Name(isolate, "console"),
			ExecuteString(isolate, context, script_console)
		);
		g->Set(
			ToV8Name(isolate, "Reflect"),
			ExecuteString(isolate, context, script_reflect_metadata)
		);
		g->Set(
			ToV8Name(isolate, "define"),
			v8::FunctionTemplate::New(isolate, &FJScriptContext::ModuleDefine, v8::External::New(v8::Isolate::GetCurrent(), retval))->GetFunction(context).ToLocalChecked()
		);

		context->SetEmbedderData(1, v8::External::New(isolate, retval));
	}

	return retval;
}

void JScriptEngine::FreeContext(IJScriptContext* Context)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	delete (FJScriptContext*)(Context);
}

FJScriptContext::FJScriptContext(v8::Local<v8::Context>& context)
{
	Context.Reset(isolate, context);
}

FJScriptContext::~FJScriptContext()
{
	for (auto& Elem : ClassMap)
	{
		delete Elem.Value;
	}
	ClassMap.Empty();

	for (auto& Elem : Modules)
	{
		delete Elem.Value;
	}
	Modules.Empty();

	Context.Reset();
}

void FJScriptContext::Expose(UClass* Class)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = Context.Get(isolate);
	v8::Context::Scope context_scope(context);

	v8::Local<v8::Function> Function = GetUClass_Function(Class);
	if (!Function.IsEmpty())
	{
		FString ClassName;
		Class->GetName(ClassName);
		v8::Local<v8::Object> g = context->Global();
		g->Set(ToV8Name(isolate, TCHAR_TO_UTF8(Class)), Function);
	}
}

void FJScriptContext::Expose(const FString& Name, UObject* Object)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = Context.Get(isolate);
	v8::Context::Scope context_scope(context);

	v8::Local<v8::Value> result = ConvertValue(Object);
	if (!result.IsEmpty())
	{
		v8::Local<v8::Object> g = context->Global();
		g->Set(ToV8String(isolate, TCHAR_TO_UTF8(*Name)), result);
	}
}

void FJScriptContext::Expose(const FString& Name, const FString& Script)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = Context.Get(isolate);
	v8::Context::Scope context_scope(context);

	v8::Local<v8::Value> result = ExecuteString(isolate, context, TCHAR_TO_UTF8(*Script));
	if (!result.IsEmpty())
	{
		v8::Local<v8::Object> g = context->Global();
		g->Set(ToV8String(isolate, TCHAR_TO_UTF8(*Name)), result);
	}
}

bool FJScriptContext::Execute(const FString& Script)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = Context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::Local<v8::Value> result = ExecuteString(isolate, context, TCHAR_TO_UTF8(*Script));
	return !result.IsEmpty();
}

IJScriptClass* FJScriptContext::CompileScript(const FString& Filename, const FString& SourceCode)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = Context.Get(isolate);
	v8::Context::Scope context_scope(context);
	v8::TryCatch try_catch;

	auto file = ToV8String(isolate, TCHAR_TO_UTF8(*Filename));
	auto source = ToV8String(isolate, TCHAR_TO_UTF8(*SourceCode));

	auto script = v8::Script::Compile(source, file);
	if (script.IsEmpty())
	{
		ReportException(isolate, try_catch);
		return nullptr;
	}

	auto result = script->Run(context).ToLocalChecked();
	if (result.IsEmpty())
	{
		ReportException(isolate, try_catch);
		return nullptr;
	}

	auto Exports = v8::Local<v8::Object>::Cast(result);
	if (Exports.IsEmpty())
	{
		return nullptr;
	}

	v8::Local<v8::String> defaultName = ToV8String(isolate, "default");
	auto ClassFunction = v8::Local<v8::Function>::Cast(Exports->Get(defaultName));
	if (ClassFunction.IsEmpty())
	{
		return nullptr;
	}

	auto Class = new FJScriptClass(ClassFunction);
	return Class;
}

void FJScriptContext::FreeScript(IJScriptClass* Class)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(Context.Get(isolate));

	delete (FJScriptClass*)(Class);
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

	delete (FJScriptObject*)(Object);
}

v8::Local<v8::Object> FJScriptContext::ConvertValue(UObject* Object)
{
	v8::EscapableHandleScope handle_scope(isolate);
	v8::Local<v8::ObjectTemplate> JSClass = GetUClass_ObjectTemplate(Object->GetClass());
	v8::Local<v8::Object> JSObject = JSClass->NewInstance();
	JSObject->SetInternalField(0, v8::External::New(isolate, Object));
	return handle_scope.Escape(JSObject);
}

v8::Local<v8::Object> FJScriptContext::ConvertValue(const uint8* Data, UScriptStruct* Struct)
{
	v8::Local<v8::Object> JSObject;
	return JSObject;
}

v8::Local<v8::Value> FJScriptContext::ConvertValue(const uint8* Data, UProperty* Property)
{
	if (!Data)
	{
		ThrowException(isolate, "Read property from invalid memory");
		return v8::Undefined(isolate);
	}

	if (auto IntProperty = Cast<UIntProperty>(Property))
	{
		return v8::Int32::New(isolate, IntProperty->GetPropertyValue_InContainer(Data));
	}
	else if (auto FloatProperty = Cast<UFloatProperty>(Property))
	{
		return v8::Number::New(isolate, FloatProperty->GetPropertyValue_InContainer(Data));
	}
	else if (auto BoolProperty = Cast<UBoolProperty>(Property))
	{
		return v8::Boolean::New(isolate, BoolProperty->GetPropertyValue_InContainer(Data));
	}
	else if (auto NameProperty = Cast<UNameProperty>(Property))
	{
		auto name = NameProperty->GetPropertyValue_InContainer(Data);
		return ToV8String(isolate, TCHAR_TO_UTF8(*name.ToString()));
	}
	else if (auto StrProperty = Cast<UStrProperty>(Property))
	{
		const FString& Value = StrProperty->GetPropertyValue_InContainer(Data);
		return ToV8String(isolate, TCHAR_TO_UTF8(*Value));
	}
	else if (auto TextProperty = Cast<UTextProperty>(Property))
	{
		const FText& Value = TextProperty->GetPropertyValue_InContainer(Data);
		return ToV8String(isolate, TCHAR_TO_UTF8(*Value.ToString()));
	}
	else if (auto ClassProperty = Cast<UClassProperty>(Property))
	{
		auto Class = Cast<UClass>(ClassProperty->GetPropertyValue_InContainer(Data));

		if (Class)
		{
			return GetUClass_Function(Class);
		}
		else
		{
			return v8::Null(isolate);
		}
	}
	else if (auto StructProperty = Cast<UStructProperty>(Property))
	{
		if (auto ScriptStruct = Cast<UScriptStruct>(StructProperty->Struct))
		{
			return ConvertValue(StructProperty->ContainerPtrToValuePtr<uint8>(Data), ScriptStruct);
		}
		else
		{
			UE_LOG(LogJScriptV8Plugin, Warning, TEXT("Non ScriptStruct found : %s"), *StructProperty->Struct->GetName());
			return v8::Undefined(isolate);
		}
	}
	else if (auto ArrayProperty = Cast<UArrayProperty>(Property))
	{
		FScriptArrayHelper_InContainer helper(ArrayProperty, Data);
		auto len = (uint32_t)(helper.Num());
		auto arr = v8::Array::New(isolate, len);
		auto context = isolate->GetCurrentContext();

		auto Inner = ArrayProperty->Inner;

		if (Inner->IsA(UStructProperty::StaticClass()))
		{
			uint8* ElementBuffer = (uint8*)FMemory_Alloca(Inner->GetSize());
			for (decltype(len) Index = 0; Index < len; ++Index)
			{
				Inner->InitializeValue(ElementBuffer);
				Inner->CopyCompleteValueFromScriptVM(ElementBuffer, helper.GetRawPtr(Index));
				arr->Set(Index, ConvertValue(ElementBuffer, Inner));
				Inner->DestroyValue(ElementBuffer);
			}
		}
		else
		{
			for (decltype(len) Index = 0; Index < len; ++Index)
			{
				arr->Set(Index, ConvertValue(helper.GetRawPtr(Index), Inner));
			}
		}

		return arr;
	}
	else if (auto ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		return ConvertValue(ObjectProperty->GetObjectPropertyValue_InContainer(Data));
	}
	else if (auto ByteProperty = Cast<UByteProperty>(Property))
	{
		auto Value = ByteProperty->GetPropertyValue_InContainer(Data);

		if (ByteProperty->Enum)
		{
			return ToV8String(isolate, TCHAR_TO_UTF8(*ByteProperty->Enum->GetEnumName(Value)));
		}
		else
		{
			return v8::Int32::New(isolate, Value);
		}
	}
	else if (auto SetProperty = Cast<USetProperty>(Property))
	{
		FScriptSetHelper_InContainer SetHelper(SetProperty, Data);

		auto Out = v8::Array::New(isolate);

		auto Num = SetHelper.Num();
		for (int Index = 0; Index < Num; ++Index)
		{
			auto PairPtr = SetHelper.GetElementPtr(Index);

			Out->Set(Index, ConvertValue(SetHelper.GetElementPtr(Index), SetProperty->ElementProp));
		}

		return Out;
	}
	else if (auto MapProperty = Cast<UMapProperty>(Property))
	{
		FScriptMapHelper_InContainer MapHelper(MapProperty, Data);

		auto Out = v8::Object::New(isolate);

		auto Num = MapHelper.Num();
		for (int Index = 0; Index < Num; ++Index)
		{
			uint8* PairPtr = MapHelper.GetPairPtr(Index);

			auto Key = ConvertValue(PairPtr + MapProperty->MapLayout.KeyOffset, MapProperty->KeyProp);
			auto Value = ConvertValue(PairPtr, MapProperty->ValueProp);

			Out->Set(Key, Value);
		}

		return Out;
	}

	return ToV8String(isolate, "<Unsupported type>");
}

bool FJScriptContext::ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UProperty* Property)
{
	if (!Data)
	{
		ThrowException(isolate, "Write property from invalid memory");
		return false;
	}

	if (auto IntProperty = Cast<UIntProperty>(Property))
	{
		auto Value = v8::Local<v8::Int32>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		IntProperty->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto FloatProperty = Cast<UFloatProperty>(Property))
	{
		auto Value = v8::Local<v8::Number>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		FloatProperty->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto BoolProperty = Cast<UBoolProperty>(Property))
	{
		auto Value = v8::Local<v8::Boolean>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		BoolProperty->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto NameProperty = Cast<UNameProperty>(Property))
	{
		auto Value = v8::Local<v8::String>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		v8::String::Utf8Value str(Value);
		NameProperty->SetPropertyValue_InContainer(Data, FName(UTF8_TO_TCHAR(*str)));
		return true;
	}
	else if (auto StrProperty = Cast<UStrProperty>(Property))
	{
		auto Value = v8::Local<v8::String>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		v8::String::Utf8Value str(Value);
		StrProperty->SetPropertyValue_InContainer(Data, UTF8_TO_TCHAR(*str));
		return true;
	}
	else if (auto TextProperty = Cast<UTextProperty>(Property))
	{
		auto Value = v8::Local<v8::String>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		v8::String::Utf8Value str(Value);
		TextProperty->SetPropertyValue_InContainer(Data, FText::FromString(UTF8_TO_TCHAR(*str)));
		return true;
	}
	else if (auto ClassProperty = Cast<UClassProperty>(Property))
	{
		if (JSValue->IsString())
		{
			v8::String::Utf8Value str(v8::Local<v8::String>::Cast(JSValue));
			auto Class = GetUClassByName(UTF8_TO_TCHAR(*str));
			if (Class)
			{
				ClassProperty->SetPropertyValue_InContainer(Data, Class);
				return true;
			}
		}

		v8::Local<v8::Object> Prototype;
		if (JSValue->IsFunction())
		{
			auto Function = v8::Local<v8::Function>::Cast(JSValue);
			if (!Function.IsEmpty())
			{
				Prototype = v8::Local<v8::Object>::Cast(Function->Get(ToV8String(isolate, "prototype")));
			}
		}
		else if (JSValue->IsObject())
		{
			auto Object = v8::Local<v8::Object>::Cast(JSValue);
			if (!Object.IsEmpty())
			{
				Prototype = v8::Local<v8::Object>::Cast(Object->GetPrototype());
			}
		}

		if (!Prototype.IsEmpty())
		{
			auto Constructor = v8::Local<v8::Function>::Cast(Prototype->Get(ToV8String(isolate, "constructor")));
			if (!Constructor.IsEmpty())
			{
				auto Name = v8::Local<v8::String>::Cast(Constructor->Get(ToV8String(isolate, "name")));
				if (!Name.IsEmpty())
				{
					v8::String::Utf8Value str(Name);
					auto Class = GetUClassByName(UTF8_TO_TCHAR(*str));
					if (Class)
					{
						ClassProperty->SetPropertyValue_InContainer(Data, Class);
						return true;
					}
				}
			}
		}
		return false;
	}
	/*
	else if (auto StructProperty = Cast<UStructProperty>(Property))
	{
		if (auto ScriptStruct = Cast<UScriptStruct>(StructProperty->Struct))
		{
			return ConvertValue(StructProperty->ContainerPtrToValuePtr<uint8>(Data), ScriptStruct);
		}
		else
		{
			UE_LOG(LogJScriptV8Plugin, Warning, TEXT("Non ScriptStruct found : %s"), *StructProperty->Struct->GetName());
			return v8::Undefined(isolate);
		}
	}
	else if (auto ArrayProperty = Cast<UArrayProperty>(Property))
	{
		FScriptArrayHelper_InContainer helper(ArrayProperty, Data);
		auto len = (uint32_t)(helper.Num());
		auto arr = v8::Array::New(isolate, len);
		auto context = isolate->GetCurrentContext();

		auto Inner = ArrayProperty->Inner;

		if (Inner->IsA(UStructProperty::StaticClass()))
		{
			uint8* ElementBuffer = (uint8*)FMemory_Alloca(Inner->GetSize());
			for (decltype(len) Index = 0; Index < len; ++Index)
			{
				Inner->InitializeValue(ElementBuffer);
				Inner->CopyCompleteValueFromScriptVM(ElementBuffer, helper.GetRawPtr(Index));
				arr->Set(Index, ConvertValue(ElementBuffer, Inner));
				Inner->DestroyValue(ElementBuffer);
			}
		}
		else
		{
			for (decltype(len) Index = 0; Index < len; ++Index)
			{
				arr->Set(Index, ConvertValue(helper.GetRawPtr(Index), Inner));
			}
		}

		return arr;
	}
	else if (auto ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		return ConvertValue(ObjectProperty->GetObjectPropertyValue_InContainer(Data));
	}
	*/
	else if (auto ByteProperty = Cast<UByteProperty>(Property))
	{
		if (ByteProperty->Enum)
		{
			if (JSValue->IsInt32())
			{
				auto EnumValue = ByteProperty->Enum->GetIndexByValue(JSValue->ToInt32()->Value());
				if (EnumValue == INDEX_NONE) return false;
				ByteProperty->SetPropertyValue_InContainer(Data, EnumValue);
			}
			else
			{
				auto Value = v8::Local<v8::String>::Cast(JSValue);
				if (Value.IsEmpty()) return false;
				v8::String::Utf8Value str(Value);
				FString Str(UTF8_TO_TCHAR(ToCString(str)));
				auto EnumValue = ByteProperty->Enum->FindEnumIndex(FName(*Str));
				if (EnumValue == INDEX_NONE) return false;
				ByteProperty->SetPropertyValue_InContainer(Data, EnumValue);
			}
		}
		else
		{
			auto Value = v8::Local<v8::Int32>::Cast(JSValue);
			if (Value.IsEmpty()) return false;
			ByteProperty->SetPropertyValue_InContainer(Data, Value->Value());
		}
		return true;
	}
	/*
	else if (auto SetProperty = Cast<USetProperty>(Property))
	{
		FScriptSetHelper_InContainer SetHelper(SetProperty, Data);

		auto Out = v8::Array::New(isolate);

		auto Num = SetHelper.Num();
		for (int Index = 0; Index < Num; ++Index)
		{
			auto PairPtr = SetHelper.GetElementPtr(Index);

			Out->Set(Index, ConvertValue(SetHelper.GetElementPtr(Index), SetProperty->ElementProp));
		}

		return Out;
	}
	else if (auto MapProperty = Cast<UMapProperty>(Property))
	{
		FScriptMapHelper_InContainer MapHelper(MapProperty, Data);

		auto Out = v8::Object::New(isolate);

		auto Num = MapHelper.Num();
		for (int Index = 0; Index < Num; ++Index)
		{
			uint8* PairPtr = MapHelper.GetPairPtr(Index);

			auto Key = ConvertValue(PairPtr + MapProperty->MapLayout.KeyOffset, MapProperty->KeyProp);
			auto Value = ConvertValue(PairPtr, MapProperty->ValueProp);

			Out->Set(Key, Value);
		}

		return Out;
	}
	*/

	return false;
}

UClass* FJScriptContext::GetUClassByName(const FString& ClassName)
{
	auto c = ClassMap.Find(ClassName);
	return c ? (*c)->Class : nullptr;
}

FJScriptContext::FV8UClass* FJScriptContext::GetUClassInfo(UClass* Class)
{
	FString ClassName;
	Class->GetName(ClassName);
	if (auto c = ClassMap.Find(ClassName)) return *c;

	auto FunctionTemplate = v8::FunctionTemplate::New(isolate);
	FunctionTemplate->SetClassName(ToV8String(isolate, TCHAR_TO_UTF8(*ClassName)));
	FunctionTemplate->InstanceTemplate()->SetInternalFieldCount(1);

	for (TFieldIterator<UProperty> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		auto Property = *It;
		FString PropertyName;
		Property->GetName(PropertyName);

		FunctionTemplate->PrototypeTemplate()->SetAccessorProperty(
			ToV8Name(isolate, TCHAR_TO_UTF8(*PropertyName)),
			v8::FunctionTemplate::New(isolate, &FJScriptContext::UClass_GetProperty, v8::External::New(isolate, Property)),
			v8::FunctionTemplate::New(isolate, &FJScriptContext::UClass_SetProperty, v8::External::New(isolate, Property))
		);
	}

	for (TFieldIterator<UFunction> It(Class, EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		FString FunctionName;
		It->GetName(FunctionName);

		if ((It->FunctionFlags & FUNC_Static))
		{
			FunctionTemplate->Set(
				ToV8String(isolate, TCHAR_TO_UTF8(*FunctionName)),
				v8::FunctionTemplate::New(isolate, &FJScriptContext::UClass_Invoke, v8::External::New(isolate, *It))
			);
		}
		else
		{
			FunctionTemplate->PrototypeTemplate()->Set(
				ToV8String(isolate, TCHAR_TO_UTF8(*FunctionName)),
				v8::FunctionTemplate::New(isolate, &FJScriptContext::UClass_Invoke, v8::External::New(isolate, *It))
			);
		}
	}

	auto V8UClass = new FV8UClass();
	V8UClass->Class = Class;
	V8UClass->FunctionTemplate.Reset(isolate, FunctionTemplate);
	V8UClass->ObjectTemplate.Reset(isolate, v8::ObjectTemplate::New(isolate, FunctionTemplate));
	V8UClass->Function.Reset(isolate, FunctionTemplate->GetFunction());
	ClassMap.Add(ClassName, V8UClass);
	return V8UClass;
}

v8::Local<v8::FunctionTemplate> FJScriptContext::GetUClass_FunctionTemplate(UClass* Class)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto ClassInfo = GetUClassInfo(Class);
	if (ClassInfo) {
		return handle_scope.Escape(ClassInfo->FunctionTemplate.Get(isolate));
	}
	return v8::Local<v8::FunctionTemplate>();
}

v8::Local<v8::ObjectTemplate> FJScriptContext::GetUClass_ObjectTemplate(UClass* Class)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto ClassInfo = GetUClassInfo(Class);
	if (ClassInfo) {
		return handle_scope.Escape(ClassInfo->ObjectTemplate.Get(isolate));
	}
	return v8::Local<v8::ObjectTemplate>();
}

v8::Local<v8::Function> FJScriptContext::GetUClass_Function(UClass* Class)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto ClassInfo = GetUClassInfo(Class);
	if (ClassInfo) {
		return handle_scope.Escape(ClassInfo->Function.Get(isolate));
	}
	return v8::Local<v8::Function>();
}

void FJScriptContext::ModuleDefine(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

	if (args.Length() != 3) return;
	if (!args[0]->IsString()) return;
	if (!args[1]->IsArray()) return;
	if (!args[2]->IsFunction()) return;

	auto context = (FJScriptContext*)(v8::Local<v8::External>::Cast(args.Data())->Value());
	if (!context) return;

	auto id = v8::Local<v8::String>::Cast(args[0]);
	if (id.IsEmpty()) return;
	auto depends = v8::Local<v8::Array>::Cast(args[1]);
	if (depends.IsEmpty()) return;
	auto factory = v8::Local<v8::Function>::Cast(args[2]);
	if (factory.IsEmpty()) return;

	auto require = v8::Object::New(args.GetIsolate());
	auto exports = v8::Object::New(args.GetIsolate());

	v8::Local<v8::Value> callArgs[30];
	if (depends->Length() > sizeof(callArgs) / sizeof(callArgs[0]))
	{
		args.GetIsolate()->ThrowException(v8::Exception::Error(ToV8String(args.GetIsolate(), "too many dependencies")));
		return;
	}

	for (uint32_t i = 0; i < depends->Length(); i++) {
		v8::String::Utf8Value name(depends->Get(i));
		if (strcmp(*name, "require") == 0)
		{
			callArgs[i] = require;
		}
		else if (strcmp(*name, "exports") == 0)
		{
			callArgs[i] = exports;
		}
		else
		{
			auto Module = context->Modules.Find(UTF8_TO_TCHAR(*name));
			if (Module)
			{
				callArgs[i] = (*Module)->Exports.Get(args.GetIsolate());
			}
			else
			{
				callArgs[i] = v8::Null(args.GetIsolate());
			}
		}
	}

	v8::Local<v8::Value> This = v8::Null(args.GetIsolate());
	v8::Local<v8::Value> retval = factory->Call(This, depends->Length(), callArgs);
	if (retval.IsEmpty()) return;

	auto Module = new FV8Module();
	Module->Name = UTF8_TO_TCHAR(*id);
	Module->Exports.Reset(args.GetIsolate(), exports);
	context->Modules.Add(Module->Name, Module);
}

void FJScriptContext::UClass_SetProperty(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::HandleScope handle_scope(isolate);
	if (args.Length() == 1)
	{
		auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
		auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
		if (Context)
		{
			auto This = (UObject*)v8::External::Cast(*args.This()->GetInternalField(0))->Value();
			auto Property = (UProperty*)v8::External::Cast(*args.Data())->Value();
			if (This && Property && This->IsValidLowLevel() && Property->IsValidLowLevel()) {
				auto Offset = Property->GetOffset_ForInternal();
				auto JSValue = args[0];
				if (!Context->ConvertJSValue(JSValue, (uint8*)This, Property))
				{
					ThrowException(isolate, "SetProperty failed");
				}
			}
		}
	}
}

void FJScriptContext::UClass_GetProperty(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::HandleScope handle_scope(isolate);
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	if (Context)
	{
		auto This = (UObject*)v8::External::Cast(*args.This()->GetInternalField(0))->Value();
		auto Property = (UProperty*)v8::External::Cast(*args.Data())->Value();
		if (This && Property && This->IsValidLowLevel() && Property->IsValidLowLevel()) {
			v8::Local<v8::Value> ReturnValue = Context->ConvertValue((const uint8_t*)This, Property);
			if (ReturnValue.IsEmpty())
			{
				ThrowException(isolate, "GetProperty failed");
			}
			else
			{
				args.GetReturnValue().Set(ReturnValue);
			}
		}
	}
}

void FJScriptContext::UClass_Invoke(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::HandleScope handle_scope(isolate);
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto Function = (UFunction*)v8::External::Cast(*args.Data())->Value();
	if (Function && Function->IsValidLowLevel())
	{
		UObject* This;
		if (Function->FunctionFlags & FUNC_Static)
		{
			This = Function->GetClass()->GetDefaultObject();
		}
		else
		{
			This = (UObject*)v8::External::Cast(*args.This()->GetInternalField(0))->Value();
		}

		uint8* Data = (uint8*)FMemory_Alloca(Function->ParmsSize);

		int32 ArgIndex = 0;
		UProperty* ReturnProperty = nullptr;
		bool bOutParams = false;
		for (TFieldIterator<UProperty> It(Function); It; ++It)
		{
			if ((It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) != CPF_Parm)
			{
				ReturnProperty = *It;
				return;
			}

			if (ArgIndex >= args.Length())
			{
				ThrowException(isolate, "");
				return;
			}

			v8::Local<v8::Value> arg = args[ArgIndex++];
			if (!Context->ConvertJSValue(arg, Data, *It))
			{
				ThrowException(isolate, "");
				return;
			}

			if ((It->PropertyFlags & (CPF_ConstParm | CPF_OutParm)) == CPF_OutParm)
			{
				bOutParams = true;
			}
		}

		This->ProcessEvent(Function, Data);

		if (ArgIndex < args.Length() && bOutParams)
		{
			auto RetVal = v8::Local<v8::Object>::Cast(args[ArgIndex]);
			if (RetVal.IsEmpty())
			{
				ThrowException(isolate, "");
				return;
			}

			for (TFieldIterator<UProperty> It(Function); It; ++It)
			{
				if ((It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) != CPF_Parm)
				{
					continue;
				}
				if ((It->PropertyFlags & (CPF_ConstParm | CPF_OutParm)) != CPF_OutParm)
				{
					continue;
				}

				auto JSValue = Context->ConvertValue(Data, *It);
				if (JSValue.IsEmpty())
				{
					ThrowException(isolate, "");
				}
				RetVal->Set(ToV8String(isolate, TCHAR_TO_UTF8(*(It->GetFName().ToString()))), JSValue);
			}

			if (ReturnProperty)
			{
				auto ReturnValue = Context->ConvertValue(Data, ReturnProperty);
				if (!ReturnValue.IsEmpty())
				{
					ThrowException(isolate, "");
					return;
				}
				RetVal->Set(ToV8String(isolate, TCHAR_TO_UTF8("$")), ReturnValue);
			}

			args.GetReturnValue().Set(RetVal);
		}
		else
		{
			if (ReturnProperty)
			{
				auto ReturnValue = Context->ConvertValue(Data, ReturnProperty);
				if (!ReturnValue.IsEmpty())
				{
					ThrowException(isolate, "");
					return;
				}
				args.GetReturnValue().Set(ReturnValue);
			}

		}
	}
}
