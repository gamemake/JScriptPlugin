#pragma once

void native_print(const v8::FunctionCallbackInfo<v8::Value>& args);
void native_abort(const v8::FunctionCallbackInfo<v8::Value>& args);
void native_trace(const v8::FunctionCallbackInfo<v8::Value>& args);
void native_profile(const v8::FunctionCallbackInfo<v8::Value>& args);

void tarray_constructor(const v8::FunctionCallbackInfo<v8::Value>& args);
void tarray_empty(const v8::FunctionCallbackInfo<v8::Value>& args);
void tarray_get(const v8::FunctionCallbackInfo<v8::Value>& args);
void tarray_set(const v8::FunctionCallbackInfo<v8::Value>& args);
void tarray_num(const v8::FunctionCallbackInfo<v8::Value>& args);
void tarray_push(const v8::FunctionCallbackInfo<v8::Value>& args);
void tarray_pop(const v8::FunctionCallbackInfo<v8::Value>& args);
void tarray_foreach(const v8::FunctionCallbackInfo<v8::Value>& args);

void tmap_constructor(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_empty(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_add(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_remove(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_set(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_find(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_foreach(const v8::FunctionCallbackInfo<v8::Value>& args);

void tmap_constructor(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_empty(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_add(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_remove(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_exists(const v8::FunctionCallbackInfo<v8::Value>& args);
void tmap_foreach(const v8::FunctionCallbackInfo<v8::Value>& args);
