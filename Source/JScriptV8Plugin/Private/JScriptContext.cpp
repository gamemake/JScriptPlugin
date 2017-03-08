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
	v8::Context::Scope context_scope(context);
	return new FJScriptContext(context);
}

void JScriptEngine::FreeContext(IJScriptContext* Context)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	delete (FJScriptContext*)(Context);
}

FJScriptContext::FJScriptContext(v8::Local<v8::Context>& context)
{
	v8::HandleScope handle_scope(isolate);

	v8::Local<v8::Object> g = context->Global();
	g->Set(
		ToV8Name(isolate, "console"),
		ExecuteString(isolate, context, script_console)
	);
	g->Set(
		ToV8Name(isolate, "define"),
		v8::FunctionTemplate::New(isolate, &FJScriptContext::ModuleDefine, v8::External::New(v8::Isolate::GetCurrent(), this))->GetFunction(context).ToLocalChecked()
	);
	g->Set(
		ToV8Name(isolate, "registerClass"),
		v8::FunctionTemplate::New(isolate, &FJScriptContext::RegisterJSClass, v8::External::New(v8::Isolate::GetCurrent(), this))->GetFunction(context).ToLocalChecked()
	);

	auto BlueprintJs = v8::Local<v8::Object>::Cast(ExecuteString(isolate, context, script_blueprint));
	if (!BlueprintJs.IsEmpty())
	{
		auto BlueprintFunc = v8::Local<v8::Function>::Cast(BlueprintJs->Get(ToV8String(isolate, "blueprint")));
		auto UpdateBPInfoFunc = v8::Local<v8::Function>::Cast(BlueprintJs->Get(ToV8String(isolate, "updateBPInfo")));
		if (!BlueprintFunc.IsEmpty() && !UpdateBPInfoFunc.IsEmpty())
		{
			g->Set(
				ToV8Name(isolate, "blueprint"),
				BlueprintFunc
			);
			UpdateBPInfo.Reset(isolate, UpdateBPInfoFunc);
		}
	}

	auto ArrayTemplate = v8::FunctionTemplate::New(isolate);
	ArrayTemplate->SetClassName(ToV8String(isolate, "TArray"));
	ArrayTemplate->InstanceTemplate()->SetInternalFieldCount(2);
	ArrayTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Empty"),	v8::FunctionTemplate::New(isolate, TArray_Empty));
	ArrayTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Get"),		v8::FunctionTemplate::New(isolate, TArray_Get));
	ArrayTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Set"),		v8::FunctionTemplate::New(isolate, TArray_Set));
	ArrayTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Num"),		v8::FunctionTemplate::New(isolate, TArray_Num));
	ArrayTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Push"),	v8::FunctionTemplate::New(isolate, TArray_Push));
	ArrayTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Pop"),		v8::FunctionTemplate::New(isolate, TArray_Pop));
	ArrayTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "ForEach"),	v8::FunctionTemplate::New(isolate, TArray_Foreach));
	TArrayTemplate.Reset(isolate, v8::ObjectTemplate::New(isolate, ArrayTemplate));

	auto MapTemplate = v8::FunctionTemplate::New(isolate);
	MapTemplate->SetClassName(ToV8String(isolate, "TMap"));
	MapTemplate->InstanceTemplate()->SetInternalFieldCount(2);
	MapTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Empty"),		v8::FunctionTemplate::New(isolate, TMap_Empty));
	MapTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Get"),		v8::FunctionTemplate::New(isolate, TMap_Get));
	MapTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Set"),		v8::FunctionTemplate::New(isolate, TMap_Set));
	MapTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "ForEach"),	v8::FunctionTemplate::New(isolate, TMap_Foreach));
	TMapTemplate.Reset(isolate, v8::ObjectTemplate::New(isolate, MapTemplate));

	auto SetTemplate = v8::FunctionTemplate::New(isolate);
	SetTemplate->SetClassName(ToV8String(isolate, "TSet"));
	SetTemplate->InstanceTemplate()->SetInternalFieldCount(2);
	SetTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Empty"),		v8::FunctionTemplate::New(isolate, TSet_Empty));
	SetTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Add"),		v8::FunctionTemplate::New(isolate, TSet_Add));
	SetTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Remove"),	v8::FunctionTemplate::New(isolate, TSet_Remove));
	SetTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "Exists"),	v8::FunctionTemplate::New(isolate, TSet_Exists));
	SetTemplate->PrototypeTemplate()->Set(ToV8String(isolate, "ForEach"),	v8::FunctionTemplate::New(isolate, TSet_Foreach));
	TSetTemplate.Reset(isolate, v8::ObjectTemplate::New(isolate, SetTemplate));

	context->SetEmbedderData(1, v8::External::New(isolate, this));
	Context.Reset(isolate, context);
}

FJScriptContext::~FJScriptContext()
{
	for (auto& Elem : UClassMap)
	{
		delete Elem.Value;
	}
	UClassMap.Empty();

	for (auto& Elem : JSClassMap)
	{
		delete Elem.Value;
	}
	JSClassMap.Empty();

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

	auto ClassInfo = GetUClassInfo(Class);
	if (ClassInfo)
	{
		v8::Local<v8::Object> g = context->Global();
		auto ClassName = GetClassName(Class);
		auto Function = ClassInfo->Function.Get(isolate);
		g->Set(ToV8Name(isolate, TCHAR_TO_UTF8(*ClassName)), Function);
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

IJScriptClass* FJScriptContext::CompileScript(const FString& Filename)
{
	FString SourceCode;
	if (!FFileHelper::LoadFileToString(SourceCode, *Filename))
	{
		return nullptr;
	}

	auto JSClass = CompileScript(Filename, SourceCode);
	if (JSClass && Filename.EndsWith(TEXT(".js")))
	{
		FString DefFile, DefText;
		DefFile = Filename.Mid(0, Filename.Len() - 3) + ".d.ts";
		if (FFileHelper::LoadFileToString(DefText, *DefFile))
		{
			v8::Isolate::Scope isolate_scope(isolate);
			v8::HandleScope handle_scope(isolate);
			v8::Local<v8::Context> context = Context.Get(isolate);
			v8::Context::Scope context_scope(context);
			v8::TryCatch try_catch;
			v8::Local<v8::Function> Function = UpdateBPInfo.Get(isolate);
			v8::Local<v8::Value> Args[1];
			Args[0] = ToV8String(isolate, DefText);
			auto retval = Function->Call(context, v8::Undefined(isolate), 1, Args);
			if (retval.IsEmpty())
			{
				ReportException(isolate, try_catch);
			}
		}
	}
	return JSClass;
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

	auto Default = Exports->Get(ToV8String(isolate, "default"));
	if (Default.IsEmpty())
	{
		return nullptr;
	}
	auto ClassFunction = v8::Local<v8::Function>::Cast(Default);
	if (ClassFunction.IsEmpty())
	{
		return nullptr;
	}

	auto ClassName = ToUEString(ClassFunction->Get(ToV8String(isolate, "name")));
	return GetJSClass(ClassName);
}

void FJScriptContext::FreeScript(IJScriptClass* Class)
{
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Context::Scope context_scope(Context.Get(isolate));

	delete (FJScriptClass*)(Class);
}

IJScriptClass* FJScriptContext::GetJSClass(const FString& ClassName)
{
	auto it = JSClassMap.Find(ClassName);
	if (it)
	{
		return *it;
	}
	return nullptr;
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

v8::Local<v8::Value> FJScriptContext::ConvertValue(const uint8* Data, UProperty* Property)
{
	if (!Data)
	{
		ThrowException(isolate, "Read property from invalid memory");
		return v8::Undefined(isolate);
	}

	if (auto Int8Property = Cast<UInt8Property>(Property))
	{
		return v8::Int32::New(isolate, Int8Property->GetPropertyValue_InContainer(Data));
	}
	else if (auto Int16Property = Cast<UInt16Property>(Property))
	{
		return v8::Int32::New(isolate, Int16Property->GetPropertyValue_InContainer(Data));
	}
	else if (auto IntProperty = Cast<UIntProperty>(Property))
	{
		return v8::Int32::New(isolate, IntProperty->GetPropertyValue_InContainer(Data));
	}
	else if (auto Int64Property = Cast<UInt64Property>(Property))
	{
		return v8::Int32::New(isolate, Int64Property->GetPropertyValue_InContainer(Data));
	}
	else if (auto ByteProperty = Cast<UByteProperty>(Property))
	{
		return v8::Int32::New(isolate, ByteProperty->GetPropertyValue_InContainer(Data));
	}
	else if (auto UInt16Property = Cast<UUInt16Property>(Property))
	{
		return v8::Int32::New(isolate, Int16Property->GetPropertyValue_InContainer(Data));
	}
	else if (auto UInt32Property = Cast<UUInt32Property>(Property))
	{
		return v8::Int32::New(isolate, IntProperty->GetPropertyValue_InContainer(Data));
	}
	else if (auto UInt64Property = Cast<UUInt64Property>(Property))
	{
		return v8::Int32::New(isolate, Int64Property->GetPropertyValue_InContainer(Data));
	}
	else if (auto FloatProperty = Cast<UFloatProperty>(Property))
	{
		return v8::Number::New(isolate, FloatProperty->GetPropertyValue_InContainer(Data));
	}
	else if (auto DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		return v8::Number::New(isolate, DoubleProperty->GetPropertyValue_InContainer(Data));
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
		return ConvertValue(Data, ClassProperty);
	}
	else if (auto ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		return ConvertValue(ObjectProperty->GetObjectPropertyValue_InContainer(Data));
	}
	else if (auto StructProperty = Cast<UStructProperty>(Property))
	{
		return ConvertValue(Data, StructProperty);
	}
	else if (auto ArrayProperty = Cast<UArrayProperty>(Property))
	{
		return ConvertValue(Data, ArrayProperty);
	}
	else if (auto SetProperty = Cast<USetProperty>(Property))
	{
		return ConvertValue(Data, SetProperty);
	}
	else if (auto MapProperty = Cast<UMapProperty>(Property))
	{
		return ConvertValue(Data, MapProperty);
	}

	return ToV8String(isolate, "<Unsupported type>");
}

v8::Local<v8::Value> FJScriptContext::ConvertValue(UObject* Object)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto ClassInfo = GetUClassInfo(Object->GetClass());
	if (ClassInfo)
	{
		auto JSClass = ClassInfo->ObjectTemplate.Get(isolate);
		v8::Local<v8::Object> JSObject = JSClass->NewInstance();
		JSObject->SetInternalField(0, v8::External::New(isolate, Object));
		return handle_scope.Escape(JSObject);
	}
	return v8::Undefined(isolate);
}

v8::Local<v8::Value> FJScriptContext::ConvertValue(const uint8* Data, UClassProperty* ClassProperty)
{
	auto Class = Cast<UClass>(ClassProperty->GetPropertyValue_InContainer(Data));
	if (Class)
	{
		auto ClassInfo = GetUClassInfo(Class);
		if (ClassInfo)
		{
			return ClassInfo->Function.Get(isolate);
		}
	}
	return v8::Undefined(isolate);
}

v8::Local<v8::Value> FJScriptContext::ConvertValue(const uint8* Data, UStructProperty* StructProperty)
{
	auto ScriptStruct = Cast<UScriptStruct>(StructProperty->Struct);
	if (!ScriptStruct)
	{
		UE_LOG(LogJScriptV8Plugin, Warning, TEXT("Non ScriptStruct found : %s"), *StructProperty->Struct->GetName());
		return v8::Undefined(isolate);
	}
	auto StructInfo = GetUStructInfo(ScriptStruct);
	if (!StructInfo)
	{
		UE_LOG(LogJScriptV8Plugin, Warning, TEXT("Non ScriptStruct found : %s"), *StructProperty->Struct->GetName());
		return v8::Undefined(isolate);
	}

	v8::EscapableHandleScope handle_scope(isolate);
	auto JSStruct = StructInfo->ObjectTemplate.Get(isolate);
	v8::Local<v8::Object> JSObject = JSStruct->NewInstance();
	JSObject->SetInternalField(0, v8::External::New(isolate, (void*)Data));
	return handle_scope.Escape(JSObject);
}

v8::Local<v8::Value> FJScriptContext::ConvertValue(const uint8* Data, UArrayProperty* ArrayProperty)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto RetVal = TArrayTemplate.Get(isolate)->NewInstance(Context.Get(isolate)).ToLocalChecked();
	RetVal->SetInternalField(0, v8::External::New(isolate, (void*)Data));
	RetVal->SetInternalField(1, v8::External::New(isolate, (void*)ArrayProperty));
	return handle_scope.Escape(RetVal);
}

v8::Local<v8::Value> FJScriptContext::ConvertValue(const uint8* Data, USetProperty* SetProperty)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto RetVal = TSetTemplate.Get(isolate)->NewInstance(Context.Get(isolate)).ToLocalChecked();
	RetVal->SetInternalField(0, v8::External::New(isolate, (void*)Data));
	RetVal->SetInternalField(1, v8::External::New(isolate, (void*)SetProperty));
	return handle_scope.Escape(RetVal);
}

v8::Local<v8::Value> FJScriptContext::ConvertValue(const uint8* Data, UMapProperty* MapProperty)
{
	v8::EscapableHandleScope handle_scope(isolate);
	auto RetVal = TMapTemplate.Get(isolate)->NewInstance(Context.Get(isolate)).ToLocalChecked();
	RetVal->SetInternalField(0, v8::External::New(isolate, (void*)Data));
	RetVal->SetInternalField(1, v8::External::New(isolate, (void*)MapProperty));
	return handle_scope.Escape(RetVal);
}

bool FJScriptContext::ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UProperty* Property)
{
	if (!Data)
	{
		ThrowException(isolate, "Write property from invalid memory");
		return false;
	}

	if (auto Int8Property = Cast<UInt8Property>(Property))
	{
		auto Value = v8::Local<v8::Int32>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		Int8Property->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto Int16Property = Cast<UInt16Property>(Property))
	{
		auto Value = v8::Local<v8::Int32>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		Int16Property->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	if (auto IntProperty = Cast<UIntProperty>(Property))
	{
		auto Value = v8::Local<v8::Int32>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		IntProperty->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto Int64Property = Cast<UInt64Property>(Property))
	{
		auto Value = v8::Local<v8::Int32>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		Int64Property->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto ByteProperty = Cast<UByteProperty>(Property))
	{
		if (ByteProperty->Enum)
		{
			if (!JSValue->IsInt32())
			{
				return false;
			}
			auto EnumValue = ByteProperty->Enum->GetIndexByValue(JSValue->ToInt32()->Value());
			if (EnumValue == INDEX_NONE) return false;
			ByteProperty->SetPropertyValue_InContainer(Data, EnumValue);
		}
		else
		{
			auto Value = v8::Local<v8::Int32>::Cast(JSValue);
			if (Value.IsEmpty()) return false;
			ByteProperty->SetPropertyValue_InContainer(Data, Value->Value());
		}
		return true;
	}
	else if (auto UInt16Property = Cast<UUInt16Property>(Property))
	{
		auto Value = v8::Local<v8::Int32>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		UInt16Property->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto UInt32Property = Cast<UUInt32Property>(Property))
	{
		auto Value = v8::Local<v8::Int32>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		UInt32Property->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto UInt64Property = Cast<UUInt64Property>(Property))
	{
		auto Value = v8::Local<v8::Int32>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		UInt64Property->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto FloatProperty = Cast<UFloatProperty>(Property))
	{
		auto Value = v8::Local<v8::Number>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		FloatProperty->SetPropertyValue_InContainer(Data, Value->Value());
		return true;
	}
	else if (auto DoubleProperty = Cast<UDoubleProperty>(Property))
	{
		auto Value = v8::Local<v8::Number>::Cast(JSValue);
		if (Value.IsEmpty()) return false;
		DoubleProperty->SetPropertyValue_InContainer(Data, Value->Value());
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
		return ConvertJSValue(JSValue, Data, ClassProperty);
	}
	else if (auto ObjectProperty = Cast<UObjectPropertyBase>(Property))
	{
		UObject* Object = ConvertJSValue(JSValue);
		if (Object)
		{
			ObjectProperty->SetObjectPropertyValue_InContainer(Data, Object);
			return true;
		}
		return true;
	}
	else if (auto StructProperty = Cast<UStructProperty>(Property))
	{
		return ConvertJSValue(JSValue, Data, StructProperty);
	}
	else if (auto ArrayProperty = Cast<UArrayProperty>(Property))
	{
		return ConvertJSValue(JSValue, Data, ArrayProperty);
	}
	else if (auto SetProperty = Cast<USetProperty>(Property))
	{
		return ConvertJSValue(JSValue, Data, SetProperty);
	}
	else if (auto MapProperty = Cast<UMapProperty>(Property))
	{
		return ConvertJSValue(JSValue, Data, MapProperty);
	}

	return false;
}

UObject* FJScriptContext::ConvertJSValue(v8::Local<v8::Value>& JSValue)
{
	return nullptr;
}

bool FJScriptContext::ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UClassProperty* ClassProperty)
{
	if (JSValue->IsString())
	{
		v8::String::Utf8Value str(v8::Local<v8::String>::Cast(JSValue));
		auto ClassInfo = GetUClassByName(UTF8_TO_TCHAR(*str));
		if (ClassInfo)
		{
			ClassProperty->SetPropertyValue_InContainer(Data, ClassInfo->Class);
			return true;
		}
	}
	else
	{
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
					auto ClassInfo = GetUClassByName(UTF8_TO_TCHAR(*str));
					if (ClassInfo)
					{
						ClassProperty->SetPropertyValue_InContainer(Data, ClassInfo->Class);
						return true;
					}
				}
			}
		}
	}
	return false;
}

bool FJScriptContext::ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UStructProperty* StructProperty)
{
	auto JSObject = v8::Local<v8::Object>::Cast(JSValue);
	if (!JSObject.IsEmpty())
	{
		auto This = StructProperty->ContainerPtrToValuePtr<uint8>(Data);
		auto Struct = StructProperty->Struct;
		for (TFieldIterator<UProperty> It(Struct, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			auto Name = It->GetName();
			auto ItemValue = JSObject->Get(ToV8String(isolate, TCHAR_TO_UTF8(*Name)));
			if (!ItemValue.IsEmpty())
			{
				ConvertJSValue(ItemValue, This, *It);
			}
		}
		return true;
	}
	return false;
}

bool FJScriptContext::ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UArrayProperty* ArrayProperty)
{
	return false;
}

bool FJScriptContext::ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, USetProperty* SetProperty)
{
	return false;
}

bool FJScriptContext::ConvertJSValue(v8::Local<v8::Value>& JSValue, uint8* Data, UMapProperty* MapProperty)
{
	return false;
}

FJScriptContext::FV8UClass* FJScriptContext::GetUClassByName(const FString& ClassName)
{
	auto c = UClassMap.Find(ClassName);
	return c ? *c : nullptr;
}

FJScriptContext::FV8UClass* FJScriptContext::GetUClassInfo(UClass* Class)
{
	FString ClassName = GetClassName(Class);
	if (auto c = UClassMap.Find(ClassName)) return *c;
	
	auto FunctionTemplate = v8::FunctionTemplate::New(isolate);
	FunctionTemplate->SetClassName(ToV8String(isolate, TCHAR_TO_UTF8(*ClassName)));
	FunctionTemplate->InstanceTemplate()->SetInternalFieldCount(1);
	if (Class->GetSuperClass())
	{
		auto ParentClass = GetUClassInfo(Class->GetSuperClass());
		if (!ParentClass) return nullptr;
		FunctionTemplate->Inherit(ParentClass->FunctionTemplate.Get(isolate));
	}

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
	UClassMap.Add(ClassName, V8UClass);
	return V8UClass;
}

FJScriptContext::FV8UStruct* FJScriptContext::GetUStructByName(const FString& StructName)
{
	auto c = UStructMap.Find(StructName);
	return c ? *c : nullptr;
}

FJScriptContext::FV8UStruct* FJScriptContext::GetUStructInfo(UStruct* Struct)
{
	FString StructName = GetStructName(Struct);
	if (auto c = UStructMap.Find(StructName)) return *c;

	auto FunctionTemplate = v8::FunctionTemplate::New(isolate);
	FunctionTemplate->SetClassName(ToV8String(isolate, TCHAR_TO_UTF8(*StructName)));
	FunctionTemplate->InstanceTemplate()->SetInternalFieldCount(1);
	if (Struct->GetSuperStruct())
	{
		auto ParentStruct = GetUStructInfo(Struct->GetSuperStruct());
		if (!ParentStruct) return nullptr;
		FunctionTemplate->Inherit(ParentStruct->FunctionTemplate.Get(isolate));
	}

	for (TFieldIterator<UProperty> It(Struct, EFieldIteratorFlags::ExcludeSuper); It; ++It)
	{
		auto Property = *It;
		FString PropertyName;
		Property->GetName(PropertyName);

		FunctionTemplate->PrototypeTemplate()->SetAccessorProperty(
			ToV8Name(isolate, TCHAR_TO_UTF8(*PropertyName)),
			v8::FunctionTemplate::New(isolate, &FJScriptContext::UStruct_GetProperty, v8::External::New(isolate, Property)),
			v8::FunctionTemplate::New(isolate, &FJScriptContext::UStruct_SetProperty, v8::External::New(isolate, Property))
		);
	}

	auto V8UStruct = new FV8UStruct();
	V8UStruct->Struct = Struct;
	V8UStruct->FunctionTemplate.Reset(isolate, FunctionTemplate);
	V8UStruct->ObjectTemplate.Reset(isolate, v8::ObjectTemplate::New(isolate, FunctionTemplate));
	V8UStruct->Function.Reset(isolate, FunctionTemplate->GetFunction());
	UStructMap.Add(StructName, V8UStruct);
	return V8UStruct;
}

void FJScriptContext::ModuleDefine(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::HandleScope handle_scope(v8::Isolate::GetCurrent());

	if (args.Length() != 3 || !args[0]->IsString() || !args[1]->IsArray() || !args[2]->IsFunction())
	{
		ThrowException(isolate, "invalid parameter");
		return;
	}

	auto context = (FJScriptContext*)(v8::Local<v8::External>::Cast(args.Data())->Value());
	if (!context) {
		ThrowException(isolate, "invalid context");
		return;
	}

	auto id = v8::Local<v8::String>::Cast(args[0]);
	auto depends = v8::Local<v8::Array>::Cast(args[1]);
	auto factory = v8::Local<v8::Function>::Cast(args[2]);
	auto require = v8::Object::New(args.GetIsolate());
	auto exports = v8::Object::New(args.GetIsolate());

	v8::Local<v8::Value> callArgs[30];
	if (depends->Length() > sizeof(callArgs) / sizeof(callArgs[0]))
	{
		ThrowException(isolate, "too many dependencies");
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
	if (retval.IsEmpty())
	{
		return;
	}

	auto Module = new FV8Module();
	Module->Name = UTF8_TO_TCHAR(*id);
	Module->Exports.Reset(args.GetIsolate(), exports);
	context->Modules.Add(Module->Name, Module);

	args.GetReturnValue().Set(exports);
}

void FJScriptContext::RegisterJSClass(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::HandleScope handle_scope(isolate);

	auto context = (FJScriptContext*)(v8::Local<v8::External>::Cast(args.Data())->Value());
	if (!context)
	{
		ThrowException(isolate, "invalid context");
		return;
	}

	if (args.Length() != 3 || !args[0]->IsString() || !args[1]->IsFunction() || !args[2]->IsObject())
	{
		ThrowException(isolate, "invalid parameter");
		return;
	}

	auto JSClassName = v8::Local<v8::String>::Cast(args[0]);
	auto Constructor = v8::Local<v8::Function>::Cast(args[1]);
	auto MetaInfo = v8::Local<v8::Object>::Cast(args[2]);

	v8::String::Utf8Value JSClassNameStr(JSClassName);
	FString ClassName = ToUEString(JSClassName);
	if (context->JSClassMap.Find(ClassName))
	{
		ThrowException(isolate, "invalid JSClass already exists");
		return;
	}

	auto JSClass = new FJScriptClass(context, Constructor, MetaInfo);
	context->JSClassMap.Add(ClassName, JSClass);
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
				auto JSValue = args[0];
				if (!Context->ConvertJSValue(JSValue, (uint8*)This, Property))
				{
					ThrowException(isolate, "UClass::SetProperty failed");
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
				ThrowException(isolate, "UClass::GetProperty failed");
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
				RetVal->Set(ToV8String(isolate, "$"), ReturnValue);
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

void FJScriptContext::UStruct_SetProperty(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::HandleScope handle_scope(isolate);
	if (args.Length() == 1)
	{
		auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
		auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
		if (Context)
		{
			auto Data = (uint8*)v8::External::Cast(*args.This()->GetInternalField(0))->Value();
			auto Property = (UProperty*)v8::External::Cast(*args.Data())->Value();
			if (Data && Property && Property->IsValidLowLevel()) {
				auto JSValue = args[0];
				if (!Context->ConvertJSValue(JSValue, Data, Property))
				{
					ThrowException(isolate, "UStruct::SetProperty failed");
				}
			}
		}
	}
}

void FJScriptContext::UStruct_GetProperty(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	v8::HandleScope handle_scope(isolate);
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	if (Context)
	{
		auto Data = (uint8*)v8::External::Cast(*args.This()->GetInternalField(0))->Value();
		auto Property = (UProperty*)v8::External::Cast(*args.Data())->Value();
		if (Data && Property && Property->IsValidLowLevel()) {
			v8::Local<v8::Value> ReturnValue = Context->ConvertValue(Data, Property);
			if (ReturnValue.IsEmpty())
			{
				ThrowException(isolate, "UStruct::GetProperty failed");
			}
			else
			{
				args.GetReturnValue().Set(ReturnValue);
			}
		}
	}
}

void FJScriptContext::TArray_Empty(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UArrayProperty*)JSData->Value();

	FScriptArrayHelper_InContainer helper(Property, Data);
	helper.EmptyValues();
}

void FJScriptContext::TArray_Get(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UArrayProperty*)JSData->Value();

	FScriptArrayHelper_InContainer helper(Property, Data);
	auto JSIndex = v8::Local<v8::Int32>::Cast(args[0]);
	if (JSIndex.IsEmpty() || JSIndex->Value() < 0 || JSIndex->Value() >= helper.Num())
	{
		return;
	}

	auto RetVal = Context->ConvertValue(helper.GetRawPtr(JSIndex->Value()), Property->Inner);
	args.GetReturnValue().Set(RetVal);
}

void FJScriptContext::TArray_Set(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UArrayProperty*)JSData->Value();

	FScriptArrayHelper_InContainer helper(Property, Data);
	auto JSIndex = v8::Local<v8::Int32>::Cast(args[0]);
	auto JSValue = args[1];
	if (JSIndex.IsEmpty() || JSIndex->Value() < 0 || JSIndex->Value() >= helper.Num())
	{
		return;
	}

	Context->ConvertJSValue(JSValue, helper.GetRawPtr(JSIndex->Value()), Property->Inner);
}

void FJScriptContext::TArray_Num(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UArrayProperty*)JSData->Value();

	FScriptArrayHelper_InContainer helper(Property, Data);
	args.GetReturnValue().Set(v8::Int32::New(isolate, helper.Num()));
}

void FJScriptContext::TArray_Push(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UArrayProperty*)JSData->Value();

	FScriptArrayHelper_InContainer helper(Property, Data);
	auto JSValue = args[0];
	if (JSValue.IsEmpty())
	{
		return;
	}

	// TODO : implement
}

void FJScriptContext::TArray_Pop(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UArrayProperty*)JSData->Value();

	FScriptArrayHelper_InContainer helper(Property, Data);
	if (helper.Num() < 1)
	{
		args.GetReturnValue().Set(v8::Undefined(isolate));
	}
	else
	{
		v8::Local<v8::Value> JSValue = Context->ConvertValue(helper.GetRawPtr(helper.Num() - 1), Property->Inner);
		helper.RemoveValues(helper.Num() - 1, 1);
		args.GetReturnValue().Set(JSValue);
	}
}

void FJScriptContext::TArray_Foreach(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UArrayProperty*)JSData->Value();

	FScriptArrayHelper_InContainer helper(Property, Data);
	auto Callback = v8::Local<v8::Function>::Cast(args[0]);
	if (Callback.IsEmpty())
	{
		return;
	}

	for (int32 index = 0; index < helper.Num(); index++)
	{
		v8::Local<v8::Value> JSValue[2];
		JSValue[0] = v8::Int32::New(isolate, index);
		JSValue[1] = Context->ConvertValue(helper.GetRawPtr(index), Property->Inner);
		v8::Local<v8::Value> RetVal = Callback->Call(v8::Undefined(isolate), 2, JSValue);
		if (RetVal.IsEmpty()) break;
		auto Break = RetVal->ToBoolean();
		if (!Break.IsEmpty() && Break->Value()) break;
	}
}

void FJScriptContext::TMap_Empty(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UMapProperty*)JSData->Value();

	FScriptMapHelper_InContainer helper(Property, Data);
	helper.EmptyValues();
}

void FJScriptContext::TMap_Get(const v8::FunctionCallbackInfo<v8::Value>& args)
{
	auto JSContext = v8::Isolate::GetCurrent()->GetCurrentContext();
	auto Context = (FJScriptContext*)v8::External::Cast(*JSContext->GetEmbedderData(1))->Value();
	auto This = v8::Local<v8::Object>::Cast(args.This());
	if (!Context || This.IsEmpty())
	{
		return;
	}

	auto JSData = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	auto JSProperty = v8::Local<v8::External>::Cast(This->GetInternalField(0));
	if (JSData.IsEmpty() || JSProperty.IsEmpty())
	{
		return;
	}
	auto Data = (uint8*)JSData->Value();
	auto Property = (UMapProperty*)JSData->Value();

	FScriptMapHelper_InContainer helper(Property, Data);
}

void FJScriptContext::TMap_Set(const v8::FunctionCallbackInfo<v8::Value>& args)
{
}

void FJScriptContext::TMap_Foreach(const v8::FunctionCallbackInfo<v8::Value>& args)
{
}

void FJScriptContext::TSet_Empty(const v8::FunctionCallbackInfo<v8::Value>& args)
{
}

void FJScriptContext::TSet_Add(const v8::FunctionCallbackInfo<v8::Value>& args)
{
}

void FJScriptContext::TSet_Remove(const v8::FunctionCallbackInfo<v8::Value>& args)
{
}

void FJScriptContext::TSet_Exists(const v8::FunctionCallbackInfo<v8::Value>& args)
{
}

void FJScriptContext::TSet_Foreach(const v8::FunctionCallbackInfo<v8::Value>& args)
{
}
