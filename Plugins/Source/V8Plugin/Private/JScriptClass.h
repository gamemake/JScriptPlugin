#pragma once

class FJScriptClass : public IJScriptClass
{
    friend class FJScriptContext;

protected:
public:
	FJScriptClass(v8::Isolate* isloate);
    virtual ~FJScriptClass();

public:
    virtual const FString& GetUEClassName();
    virtual const FString& GetClassName();

    virtual TArray<FJScriptField>& GetFields();
    virtual TArray<FJScriptMethod>& GetMethods();

public:
	bool ProcessJSClass(v8::Local<v8::Value>& Class);
	
	FJScriptField* GetField(FString& Name);
    FJScriptMethod* GetMethod(FString& Name);

private:
    v8::Persistent<v8::Value> JSClass;
    FString UEClassName;
    FString ClassName;
    TArray<FJScriptField> Fields;
    TArray<FJScriptMethod> Methods;
    TMap<FString, int> FieldMap;
    TMap<FString, int> MethodMap;
};