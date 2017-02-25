#pragma once

class FJScriptClass : public IJScriptClass
{
	friend class FJScriptContext;

protected:
public:
	FJScriptClass(v8::Local<v8::Function> Function);
	virtual ~FJScriptClass();

public:
	virtual const FString& GetUEClassName();
	virtual const FString& GetClassName();

	virtual TArray<FJScriptField>& GetFields();
	virtual TArray<FJScriptMethod>& GetMethods();

public:

	FJScriptField* GetField(FString& Name);
	FJScriptMethod* GetMethod(FString& Name);

private:
	v8::Persistent<v8::Function> JSClass;
	FString UEClassName;
	FString ClassName;
	TArray<FJScriptField> Fields;
	TArray<FJScriptMethod> Methods;
	TMap<FString, int> FieldMap;
	TMap<FString, int> MethodMap;
};