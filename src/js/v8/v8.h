#ifndef MY_V8_H
#define MY_V8_H

#include "include/libplatform/libplatform.h"
#include "include/v8.h"

#include <string>
#include <unordered_map>

#include "../../config.h"

using namespace std;

namespace myv8 {

extern unique_ptr<v8::Platform> platform;
extern unordered_map<string, string> cache_by_name;
extern int self_idx;

string InitV8(char* argv[]);

class V8Instance {
public:
	myconfig::Config& conf_;
	v8::Isolate* isolate_;
	v8::Eternal<v8::Context> context_;
	unordered_map<string, v8::Local<v8::Module>> module_by_name_;
	string current_module_name_;
//	v8::Local<v8::ObjectTemplate> header_templ_;

	V8Instance(myconfig::Config& ins);
	string Init();
	string CreateModule(v8::Local<v8::Context>& context, string mname);
	string InitModule(v8::Local<v8::Context>& context, string mname);

	int self_id_;
	string SetSelfId(int new_self_id);
};
V8Instance* getCurrentInstance(v8::Local<v8::Context> context);

v8::MaybeLocal<v8::Module> ResolveModuleCallback(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::Module> referrer);
v8::MaybeLocal<v8::Promise> HostImportModuleDynamicallyCallback(v8::Local<v8::Context> context, v8::Local<v8::ScriptOrModule> referrer, v8::Local<v8::String> specifier);

string RunScript(v8::Local<v8::Context>& context, string js, string resource_name = "unknown", int resource_line_offset = 0, int resource_column_offset = 0);

void V8TrhowErr(v8::Isolate* isolate, string&& errText);
v8::Local<v8::Promise> GetPromiseReject(v8::Local<v8::Context> context, string&& err);
string GetException(v8::Local<v8::Context>& context, v8::TryCatch& tryCatch);

string StrToV8(v8::Local<v8::String>& v8Str, string_view str, v8::Isolate* isolate);
string V8ToStr(v8::Local<v8::String>& v8Str, v8::Isolate* isolate);
string V8ToStr(v8::Local<v8::String>&& v8Str, v8::Isolate* isolate);

}

#include "./v8.cc"

#endif
