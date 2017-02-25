#pragma once

const char* ToCString(const v8::String::Utf8Value& value);
v8::Local<v8::String> ToV8String(v8::Isolate* isolate, const char* value);
v8::Local<v8::Name> ToV8Name(v8::Isolate* isolate, const char* name);

bool GetClassName(v8::Isolate* isolate, v8::Local<v8::Value>& JSValue, FString& ClassName);

v8::Local<v8::String> LoadFileToString(v8::Isolate* isolate, const char* filename);
v8::Local<v8::Value> ExecuteString(v8::Isolate* isolate, v8::Local<v8::Context>& context, const char* code);
void ReportException(v8::Isolate* isolate, v8::TryCatch& try_catch);
void ThrowException(v8::Isolate* isolate, const char* Message);
