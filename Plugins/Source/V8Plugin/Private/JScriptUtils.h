#pragma once

const char* ToCString(const v8::String::Utf8Value& value);
v8::Local<v8::String> LoadFileToString(v8::Isolate* isolate, const char* filename);
v8::Local<v8::Value> ExecuteString(v8::Isolate* isolate, v8::Local<v8::Context>& context, v8::Local<v8::String>& code);
void ReportException(v8::Isolate* isolate, v8::TryCatch& try_catch);
