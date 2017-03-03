#include "JScriptV8PluginPCH.h"

#include "JScriptContext.h"
#include "JScriptClass.h"
#include "JScriptUtils.h"

FJScriptClass::FJScriptClass(FJScriptContext* context, v8::Local<v8::Function>& constructor, v8::Local<v8::Object>& metainfo)
{
	Context = context;
	Construstor.Reset(v8::Isolate::GetCurrent(), constructor);
	JSMetaInfo.Reset(v8::Isolate::GetCurrent(), metainfo);
}

FJScriptClass::~FJScriptClass()
{
	Construstor.Reset();
	JSMetaInfo.Reset();
}

const FString& FJScriptClass::GetClassName() const
{
	return MetaInfo.ClassName;
}

const FString& FJScriptClass::GetParentClassName() const
{
	return MetaInfo.ParentClassName;
}

bool FJScriptClass::LoadMetaInfo(const FJScriptClassInfo& metainfo)
{
	MetaInfo = metainfo;
	return true;
}

bool FJScriptClass::ParseJSMetaInfo()
{
	auto isolate = v8::Isolate::GetCurrent();
	v8::HandleScope handle_scope(isolate);
	auto metainfo = JSMetaInfo.Get(isolate);

	MetaInfo.ClassName = ToUEString(metainfo->Get(ToV8String(isolate, "ClassName")));
	MetaInfo.ParentClassName = ToUEString(metainfo->Get(ToV8String(isolate, "ParentClassName")));
	if (MetaInfo.ClassName.IsEmpty())
	{
		//clear
		return false;
	}
	if (!MetaInfo.ParentClassName.IsEmpty())
	{
		MetaInfo.ParentClassJS = Context->GetJSClass(MetaInfo.ParentClassName);
		if (MetaInfo.ParentClassJS)
		{
			MetaInfo.ParentClassUE = nullptr;
		}
		else
		{
			return false;
		}
	}

	auto fields = v8::Local<v8::Array>::Cast(metainfo->Get(ToV8String(isolate, "Fields")));
	auto methods = v8::Local<v8::Array>::Cast(metainfo->Get(ToV8String(isolate, "Methods")));

	MetaInfo.Fields.Empty();
	if (!fields.IsEmpty())
	{
		for (uint32_t index = 0; index < fields->Length(); index++)
		{
			auto fieldinfo = v8::Local<v8::Object>::Cast(fields->Get(index));
			if (fieldinfo.IsEmpty())
			{
				return false;
			}

			FJScriptFieldInfo Field;
			if (!ParserField(Field, fieldinfo))
			{
				return false;
			}

			MetaInfo.Fields.Add(Field);
		}
	}

	MetaInfo.Methods.Empty();
	if (!methods.IsEmpty())
	{
		for (uint32_t mindex = 0; mindex < methods->Length(); mindex++)
		{
			auto method = v8::Local<v8::Object>::Cast(methods->Get(mindex));
			if (method.IsEmpty())
			{
				return false;
			}

			auto IsStatic = v8::Local<v8::Boolean>::Cast(method->Get(ToV8String(isolate, "IsStatic")));
			auto Name = v8::Local<v8::String>::Cast(method->Get(ToV8String(isolate, "Name")));
			if (IsStatic.IsEmpty() || Name.IsEmpty())
			{
				return false;
			}

			FJScriptMethodInfo MethodInfo;
			MethodInfo.IsStatic = IsStatic->Value();
			MethodInfo.Name = ToUEString(Name);

			auto Parameters = v8::Local<v8::Array>::Cast(method->Get(ToV8String(isolate, "Parameters")));
			if (!Parameters.IsEmpty())
			{
				for (uint32_t pindex = 0; pindex < Parameters->Length(); pindex++)
				{
					auto Parameter = v8::Local<v8::Object>::Cast(Parameters->Get(pindex));
					if (Parameter.IsEmpty())
					{
						return false;
					}

					FJScriptFieldInfo Field;
					if (!ParserField(Field, Parameter))
					{
						return false;
					}

					MethodInfo.Params.Add(Field);
				}
			}

			auto ReturnValue = v8::Local<v8::Object>::Cast(method->Get(ToV8String(isolate, "ReturnValue")));
			if (!ReturnValue.IsEmpty())
			{
				if (!ParserField(MethodInfo.ReturnValue, ReturnValue))
				{
					return false;
				}
			}

			MetaInfo.Methods.Add(MethodInfo);
		}
	}

	return true;
}

const TArray<FJScriptFieldInfo>& FJScriptClass::GetFields() const
{
	return MetaInfo.Fields;
}

const TArray<FJScriptMethodInfo>& FJScriptClass::GetMethods() const
{
	return MetaInfo.Methods;
}

bool FJScriptClass::ParserField(FJScriptFieldInfo& Field, v8::Local<v8::Object>& FieldInfo)
{
	auto isolate = v8::Isolate::GetCurrent();
	v8::HandleScope handle_scope(isolate);

	auto IsStatic = v8::Local<v8::Boolean>::Cast(FieldInfo->Get(ToV8String(isolate, "IsStatic")));
	auto Name = v8::Local<v8::String>::Cast(FieldInfo->Get(ToV8String(isolate, "Name")));
	auto IsArray = v8::Local<v8::Boolean>::Cast(FieldInfo->Get(ToV8String(isolate, "IsArray")));
	auto Type = v8::Local<v8::String>::Cast(FieldInfo->Get(ToV8String(isolate, "Type")));
	if (IsStatic.IsEmpty() || Name.IsEmpty() || IsArray.IsEmpty() || Type.IsEmpty())
	{
		return false;
	}

	Field.IsStatic = IsStatic->Value();
	Field.Name = ToUEString(Name);
	Field.IsArray = IsArray->Value();
	Field.Type = ToUEString(Type);

	if (Field.Type == TEXT("Number"))
	{
		Field.PropertyClass = UFloatProperty::StaticClass();
	}
	else if (Field.Type == TEXT("String"))
	{
		Field.PropertyClass = UStrProperty::StaticClass();
	}
	else if (Field.Type == TEXT("Boolean"))
	{
		Field.PropertyClass = UBoolProperty::StaticClass();
	}
	else
	{
		Field.PropertyClass = nullptr;
		Field.JSClass = Context->GetJSClass(Field.Name);
		if (!Field.JSClass)
		{
			return false;
		}
	}

	return true;
}
