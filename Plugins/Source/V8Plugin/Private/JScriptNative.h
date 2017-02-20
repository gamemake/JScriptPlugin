#pragma once

void native_print(const v8::FunctionCallbackInfo<v8::Value>& args);
void native_abort(const v8::FunctionCallbackInfo<v8::Value>& args);
void native_trace(const v8::FunctionCallbackInfo<v8::Value>& args);
void native_profile(const v8::FunctionCallbackInfo<v8::Value>& args);
