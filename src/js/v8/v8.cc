#include "include/libplatform/libplatform.h"
#include "include/v8.h"

#include <iostream>
#include <string>

#include "../../config.h"
#include "../../file.h"
#include "../../log.h"
#include "../../url.h"
#include "../js.h"

#include "./api/fetch.h"
#include "./api/log.h"

#include "./v8.h"

using namespace std;

namespace myv8 {

unique_ptr<v8::Platform> platform;
unordered_map<string, string> cache_by_name;
int self_idx = 0;

string GetErr(string err, string_view func, v8::Local<v8::Context>& context, v8::TryCatch& try_catch) {
	string ex = myv8::GetException(context, try_catch);
	if (ex != "") {
		err += ">>" + ex;
	}
	return mylog::GetErr(err, func);
}

string InitV8(char* argv[]) {
	auto execPath = argv[0];
	v8::V8::InitializeICUDefaultLocation(execPath);
	v8::V8::InitializeExternalStartupData(execPath);
	platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(platform.get());
	v8::V8::Initialize();
	return "";
}

V8Instance::V8Instance(myconfig::Config& conf) : conf_(conf), self_id_(++self_idx) {
}
string V8Instance::Init() {
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	isolate_ = v8::Isolate::New(create_params);
	v8::Isolate::Scope isolate_scope(isolate_);

//	isolate_->SetMicrotasksPolicy(v8::MicrotasksPolicy::kExplicit);
	isolate_->SetHostImportModuleDynamicallyCallback(HostImportModuleDynamicallyCallback);
	// setting this callback enables import.meta
//	isolate_->SetHostInitializeImportMetaObjectCallback([](v8::Local<v8::Context> context, v8::Local<Module> module, v8::Local<v8::Object> meta) {
		// meta->Set(key, value); you could set import.meta.url here
//	});
	v8::HandleScope handle_scope(isolate_);
	v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate_);
//API
//todo сделать обход по конфигу расширений и вызова функции подключения - макрс
	v8::Local<v8::String> log_func_name;
	string err = StrToV8(log_func_name, "__ccLog", isolate_);
	if (err != "") {
		return err;
	}
	global->Set(log_func_name, v8::FunctionTemplate::New(isolate_, log::LogCallback));

	v8::Local<v8::String> fetch_func_name;
	err = StrToV8(fetch_func_name, "__ccFetch", isolate_);
	if (err != "") {
		return err;
	}
	global->Set(fetch_func_name, v8::FunctionTemplate::New(isolate_, fetch::FetchCallback));

	v8::Local<v8::Context> context = v8::Context::New(isolate_, nullptr, global);
	context->SetAlignedPointerInEmbedderData(100, this);
	context_.Set(isolate_, context);
	return "";
}
string V8Instance::CreateModule(v8::Local<v8::Context>& context, string mname) {
	if (module_by_name_.count(mname) != 0) {
		return "";
	}
	v8::TryCatch try_catch(isolate_);
	unordered_map<string, bool> is_ex;
	for (const auto& m : module_by_name_) {
		is_ex[m.first] = true;
	}
	string err = InitModule(context, mname);
	if (err != "") {
		return GetErr("Create module " + mname + " error", string_view(__FUNCTION__), context, try_catch);
	}
	for (const auto& [mname, module] : module_by_name_) {
		if (is_ex.count(mname) != 0) {
			continue;
		}
		current_module_name_ = mname;
		if (module->InstantiateModule(context, ResolveModuleCallback).IsNothing()) {
			return GetErr("Create module " + mname + " error (InstantiateModule error)", string_view(__FUNCTION__), context, try_catch);
		}
	}
	auto module = module_by_name_[mname];
	v8::Local<v8::Value> result;
	if (!module->Evaluate(context).ToLocal(&result)) {
		return GetErr("Can't evaluate module " + mname, string_view(__FUNCTION__), context, try_catch);
	}
	auto result_promise = result.As<v8::Promise>();
	if (v8::Module::kEvaluated != module->GetStatus() || result_promise->State() != v8::Promise::kFulfilled) {
//todo add line
		v8::Local<v8::String> v_err;
		if (!result_promise->Result()->ToString(context).ToLocal(&v_err)) {
			return GetErr("Evaluate module " + mname + " error: can't convert reject result to string", string_view(__FUNCTION__), context, try_catch);
		}
		return GetErr("Evaluate module " + mname + " error: " + V8ToStr(v_err, isolate_), string_view(__FUNCTION__), context, try_catch);
	}
//cout << "1: " << mname << "-" << v8::Module::kEvaluated << " = " << module->GetStatus() << "\n";
//cout << "2: " << result_promise->State() << " = " << v8::Promise::kFulfilled << "\n";
//cout << "3: " << result_promise->Result()->IsUndefined() << "\n";
	return "";
}
string V8Instance::InitModule(v8::Local<v8::Context>& context, string mname) {
	if (module_by_name_.count(mname) != 0) {
		return "";
	}
	myurl::NormalizeUrl(mname);
	string js;
	if (cache_by_name.count(mname) == 0) {
		string err = myfile::ReadFile(js, mname);
		if (err != "") {
			return mylog::GetErr("Can't initModule: " + err, string_view(__FUNCTION__));
		}
		cache_by_name[mname] = js;
	} else {
		js = cache_by_name[mname];
	}
	v8::Local<v8::String> v_mname;
	string err = StrToV8(v_mname, mname, isolate_);
	if (err != "") {
		return mylog::GetErr("Can't create v8 mname " + mname, string_view(__FUNCTION__));
	}
	v8::ScriptOrigin origin(isolate_, v_mname, 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true);
	v8::Local<v8::String> v8Js;
	err = StrToV8(v8Js, js, isolate_);
	if (err != "") {
		return mylog::GetErr("Can't create module v8 source", string_view(__FUNCTION__));
	}
	v8::ScriptCompiler::Source source(v8Js, origin);
	v8::Local<v8::Module> module;
	if (!v8::ScriptCompiler::CompileModule(isolate_, &source).ToLocal(&module)) {
		return mylog::GetErr("Compilation error " + mname, string_view(__FUNCTION__));
	}
	module_by_name_[mname] = module;
	int mLen = module->GetModuleRequestsLength();
	for (int i = 0; i < mLen; i++) {
		string iMname = V8ToStr(module->GetModuleRequest(i), isolate_);
		if (module_by_name_.count(iMname) != 0) {
			continue;
		}
		myurl::NormalizeUrl(iMname, mname);
		string err = InitModule(context, iMname);
		if (err != "") {
			return mylog::GetErr("Can't InitModule (GetModuleRequestsLength)>>" + err, string_view(__FUNCTION__));
		}
	}
	return "";
}
V8Instance* getCurrentInstance(v8::Local<v8::Context> context) {
	return !context.IsEmpty() ? static_cast<V8Instance*>(context->GetAlignedPointerFromEmbedderData(100)) : nullptr;
}

v8::MaybeLocal<v8::Module> ResolveModuleCallback(v8::Local<v8::Context> context, v8::Local<v8::String> specifier, v8::Local<v8::Module> referrer) {
	v8::Isolate* isolate = context->GetIsolate();
	string mname = V8ToStr(specifier, isolate);
	auto v8ins = getCurrentInstance(context);
	if (v8ins == nullptr) {
//		V8TrhowErr(isolate, string(__FUNCTION__) + ">>Error link module " + mname + ", can't find instance");
		cout << mylog::GetErr("Error link module " + mname + ", can't find instance", string_view(__FUNCTION__)) << "\n";
		return v8::MaybeLocal<v8::Module>();
	}
	myurl::NormalizeUrl(mname, v8ins->current_module_name_);
	if (v8ins->module_by_name_.count(mname) == 0) {
//		V8TrhowErr(isolate, string(__FUNCTION__) + ">>Error link module " + mname + ", can't find by name.");
		cout << mylog::GetErr("Error link module " + mname + ", can't find by name", string_view(__FUNCTION__)) << "\n";
		return v8::MaybeLocal<v8::Module>();
	}
	return v8ins->module_by_name_[mname];
}
//todo
v8::MaybeLocal<v8::Promise> HostImportModuleDynamicallyCallback(v8::Local<v8::Context> context, v8::Local<v8::ScriptOrModule> referrer, v8::Local<v8::String> specifier) {
//v8::MaybeLocal<v8::Promise> HostImportModuleDynamicallyCallback(v8::Local<v8::Context> context, v8::Local<v8::ScriptOrModule> referrer, v8::Local<v8::String> specifier, v8::Local<v8::FixedArray> import_assertions) {
	v8::Isolate* isolate = context->GetIsolate();
	v8::EscapableHandleScope handle_scope(isolate);
	string mname = V8ToStr(specifier, isolate);
	myurl::NormalizeUrl(mname);
	auto v8ins = getCurrentInstance(context);
	if (v8ins == nullptr) {
		return GetPromiseReject(context, string(__FUNCTION__) + ">>Link dynamic module " + mname + " error. Can't find instance.");
	}
cout << "hostImportModuleDynamicallyCallback: " << mname << "\n";
	v8::TryCatch try_catch(isolate);
	string err = v8ins->CreateModule(context, mname);
	if (err != "") {
		string ex = GetException(context, try_catch);
		if (ex != "") {
			err += "\n" + ex;
		}
		return handle_scope.Escape(GetPromiseReject(context, string(__FUNCTION__) + ">>Link dynamic module " + mname + " error. Can't create: " + err));
	}
	err = GetException(context, try_catch);
	if (err != "") {
		return handle_scope.Escape(GetPromiseReject(context, string(__FUNCTION__) + ">>Link dynamic module " + mname + " error. Script exception: " + err));
	}
//	auto module_by_name_ = module_by_name_;
//	if (module_by_name_.count(mname) == 0) {
//		return handle_scope.Escape(GetPromiseReject(context, string(__FUNCTION__) + ">>Link dynamic module " + mname + " error. Can't find by name"));
//	}
	v8::Local<v8::Promise::Resolver> resolver;
	if (!v8::Promise::Resolver::New(context).ToLocal(&resolver)) {
		return handle_scope.Escape(GetPromiseReject(context, string(__FUNCTION__) + ">>Link dynamic module " + mname + " error. Can't create local resolver."));
	}
	if (!resolver->Resolve(context, v8ins->module_by_name_[mname]->GetModuleNamespace()).FromMaybe(false)) {
		return handle_scope.Escape(GetPromiseReject(context, string(__FUNCTION__) + ">>Link dynamic module " + mname + " error. Resolve to local error."));
	}
	return handle_scope.Escape(resolver->GetPromise());
}
string RunScript(v8::Local<v8::Context>& context, string js, string resource_name, int resource_line_offset, int resource_column_offset) {
	auto isolate = context->GetIsolate();
	v8::Local<v8::String> source;
	string err = StrToV8(source, js, isolate);
	if (err != "") {
		return mylog::GetErr("Can't create v8 js string", string_view(__FUNCTION__));
	}
	v8::Local<v8::String> v_resource_name;
	err = StrToV8(v_resource_name, resource_name, isolate);
	if (err != "") {
		return mylog::GetErr(err, string_view(__FUNCTION__));
	}
	v8::ScriptOrigin origin(isolate, v_resource_name, resource_line_offset, resource_column_offset);
	v8::Local<v8::Script> script;
	v8::TryCatch try_catch(isolate);
	if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
		return GetErr("Can't compile script", string_view(__FUNCTION__), context, try_catch);
	}
	v8::Local<v8::Value> result;
	if (!script->Run(context).ToLocal(&result)) {
		return GetErr("Can't run script", string_view(__FUNCTION__), context, try_catch);
	}
	return "";
}
void V8TrhowErr(v8::Isolate* isolate, string&& errText) {
	v8::Local<v8::String> v_err;
	string err = StrToV8(v_err, errText, isolate);
	if (err != "") {
		cout << mylog::GetErr(err, string_view(__FUNCTION__)) << "\n";
	}
cout << "ThrowEr " << errText << "\n";
//	isolate->ThrowException(v_err);
	isolate->ThrowError(v_err);
}
v8::Local<v8::Promise> GetPromiseReject(v8::Local<v8::Context> context, string&& rejectErr) {
	v8::Local<v8::Promise::Resolver> resolver;
	if (!v8::Promise::Resolver::New(context).ToLocal(&resolver)) {
		cout << mylog::GetErr("Can't create resolver.\n" + rejectErr, string_view(__FUNCTION__)) << "\n";
		return v8::Local<v8::Promise>();
	}
	v8::Local<v8::String> v_reject_err;
	string err = StrToV8(v_reject_err, rejectErr, context->GetIsolate());
	if (err != "") {
		cout << mylog::GetErr("Can't convert err to v8::String.\n" + rejectErr, string_view(__FUNCTION__)) << "\n";
		return v8::Local<v8::Promise>();
	}
	if (!resolver->Reject(context, v_reject_err).FromMaybe(false)) {
		cout << mylog::GetErr("Can't reject promise on err: " + rejectErr, string_view(__FUNCTION__)) << "\n";
		return v8::Local<v8::Promise>();
	}
	return resolver->GetPromise();
}
string GetException(v8::Local<v8::Context>& context, v8::TryCatch& try_catch) {
	if (!try_catch.HasCaught()) {
		return "";
	}
	v8::Isolate* isolate = context->GetIsolate();
	v8::Message* msg = *try_catch.Message();
	auto m = msg->Get();
	v8::Local<v8::String> fname;
	if (!msg->GetScriptResourceName()->ToString(context).ToLocal(&fname)) {
		return mylog::GetErr("Can't create fname.\n" + V8ToStr(m, isolate));
	}
	return mylog::GetErr("(file " + V8ToStr(fname, isolate) + ":" + to_string(msg->GetLineNumber(context).FromMaybe(0)) + ":" + to_string(msg->GetStartColumn()) + "): " + V8ToStr(m, isolate));
}
string StrToV8(v8::Local<v8::String>& v8Str, string_view str, v8::Isolate* isolate) {
	return v8::String::NewFromUtf8(isolate, str.data()).ToLocal(&v8Str) ? "" : "Can't create v8::String: " + string(str);
}
string V8ToStr(v8::Local<v8::String>& v8Str, v8::Isolate* isolate) {
//	v8::String::Utf8Value _str(isolate, v8);
//	string str(*_str, _str.length());
//	return str;
	v8::String::Utf8Value str(isolate, v8Str);
	return *str;
}
string V8ToStr(v8::Local<v8::String>&& v8Str, v8::Isolate* isolate) {
	v8::String::Utf8Value str(isolate, v8Str);
	return *str;
}
string V8Instance::SetSelfId(int new_self_id) {
	v8::HandleScope scope(isolate_);
	v8::Local<v8::Context> context = context_.Get(isolate_);
	v8::Context::Scope context_scope(context);
	int cur_id = self_id_;
	self_id_ = new_self_id;
	return RunScript(context, "__mSetSelf(" + to_string(cur_id) + ", " + to_string(new_self_id) + ")");//, resource_name, resource_line_offset, resource_column_offset);
}

}
