#pragma once

class FJScriptContext : public IJScriptContext
{
    friend class JScriptEngine;

protected:
    FJScriptContext(v8::Local<v8::Context>& Context);
    virtual ~FJScriptContext();

public:
    virtual void Expose(const FString& Name, UClass* Class) override;
	virtual void Expose(const FString& Name, UObject* Object) override;
	virtual void Expose(const FString& Name, const FString& Script) override;
	virtual void Execute(const FString& Script) override;

    virtual IJScriptClass* CompileScript(const FString& Filename, const FString& SourceCode) override;
    virtual void FreeScript(IJScriptClass* Class) override;

    virtual IJScriptObject* CreateObject(IJScriptClass* Class) override;
    virtual void FreeObject(IJScriptObject* Object) override;

    virtual void Invoke(IJScriptObject* Object, const FString& Name) override;
    virtual void SetProperty(IJScriptObject* Object, const FString& Name, const FString& Value) override;
    virtual void GetProperty(IJScriptObject* Object, const FString& Name, FString& Value) override;

private:
    v8::Persistent<v8::Context> Context;
};
