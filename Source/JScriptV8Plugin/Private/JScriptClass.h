#pragma once

class FJScriptContext;

class FJScriptClass : public IJScriptClass
{
	friend class FJScriptContext;

public:
	FJScriptClass(FJScriptContext* context, v8::Local<v8::Function>& constructor, v8::Local<v8::Object>& metainfo);
	virtual ~FJScriptClass();

public:
	bool LoadMetaInfo(const FJScriptClassInfo& metainfo);
	bool ParseJSMetaInfo();

	virtual const FString& GetClassName() const override;
	virtual const FString& GetParentClassName() const override;

	virtual const TArray<FJScriptFieldInfo>& GetFields() const override;
	virtual const TArray<FJScriptMethodInfo>& GetMethods() const override;

private:
	bool ParserField(FJScriptFieldInfo& Field, v8::Local<v8::Object>& FieldInfo);

	FJScriptContext* Context;
	v8::Persistent<v8::Function> Construstor;
	v8::Persistent<v8::Object> JSMetaInfo;

	FJScriptClassInfo MetaInfo;
};
