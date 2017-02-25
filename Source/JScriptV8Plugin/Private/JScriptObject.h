#pragma once

class FJScriptObject : public IJScriptObject
{
	friend class FJScriptContext;

protected:
	FJScriptObject(v8::Isolate* isloate, v8::Local<v8::Value> Object);
	virtual ~FJScriptObject();

public:
	virtual void Invoke(const FString& Name);
	virtual void SetProperty(const FString& Name, const FString& Value);
	virtual void GetProperty(const FString& Name, FString& Value);

private:
	v8::Persistent<v8::Value> JSObject;
};
