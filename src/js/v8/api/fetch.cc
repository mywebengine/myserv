#include <iostream>
#include <string>

#include "include/v8.h"

#include "../../../url.h"
#include "../../../log.h"
#include "../../../http/fetch.h"
#include "../../../http/http.h"
#include "../../js.h"
#include "../v8.h"
#include "./fetch.h"

using namespace std;

namespace myv8 {
namespace fetch {

void FetchCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto isolate = args.GetIsolate();

	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope scope(isolate);
	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Context::Scope context_scope(context);

//	int p_len = args.Length();
	string url;
	if (args[0]->IsString()) {
		url = V8ToStr(args[0].As<v8::String>(), isolate);
//		string opt;
//		if (p_len > 1) {
//			opt = V8ToStr(args[1].As<v8::Object>(), isolate);
//		}
//		req = url + ":" + opt;
	} else if (args[0]->IsObject()) {
		url = V8ToStr(args[0].As<v8::String>(), isolate);
	}
	v8::Local<v8::Promise::Resolver> resolver;
	if (!v8::Promise::Resolver::New(context).ToLocal(&resolver)) {
		V8TrhowErr(isolate, string(__FUNCTION__) + ">>Error in Fetch, can't create resolver");
		return;
	}
	auto fetch_args = make_shared<FetchCallbackInfo>();
	fetch_args->req = myhttp::Request {
		myurl::Parse(url)
	};
//	fetch_args->cb.Reset(isolate, args[2].As<v8::Function>());
	fetch_args->resolver.Reset(isolate, resolver);
//	fetch_args->context.Reset(isolate, context);
	auto v8ins = getCurrentInstance(isolate->GetCurrentContext());
	if (v8ins == nullptr) {
		V8TrhowErr(isolate, string(__FUNCTION__) + ">>Error in Fetch, can't find instance");
		return;
	}
	args.GetReturnValue().Set(resolver->GetPromise());
	auto self_id = v8ins->self_id_;
	myhttp::Fetch(v8ins->conf_, fetch_args->req, [v8ins, self_id, fetch_args](boost::beast::http::response<boost::beast::http::string_body>* res, string_view fetch_err) {
		string err = v8ins->SetSelfId(self_id);
		if (err != "") {
			cout << mylog::GetErr("fetch>>" + err, "FetchCallback") << "\n";
			return;
		}
		auto isolate = v8ins->isolate_;
//		v8::Isolate::Scope isolate_scope(isolate);
		v8::HandleScope scope(isolate);
		v8::Local<v8::Context> context = v8ins->context_.Get(isolate);
//		v8::Local<v8::Context> context = fetch_args->context.Get(isolate);
		v8::Context::Scope context_scope(context);
		auto resolver = fetch_args->resolver.Get(isolate);
		v8::Local<v8::String> v_key;
		v8::Local<v8::String> v_val;
		auto RejectErr = [&resolver, isolate, &context](string err) {
			v8::Local<v8::String> v_err;
			string e = StrToV8(v_err, "fetch>>" + err, isolate);
			if (e != "") {
				cout << mylog::GetErr("fetch>>" + err, "FetchCallback") << "\n";
				return;
			}
			if (!resolver->Reject(context, v_err).FromMaybe(false)) {
				cout << mylog::GetErr("fetch>>Can't reject promise on err: " + err, "FetchCallback") << "\n";
			}
		};
//fetch_err = "sdfsdfds";
		if (fetch_err != "") {
			RejectErr(fetch_err.data());
			return;
		}
		string url = fetch_args->req.url.href;
//		string err;
		v8::Local<v8::Object> v_req = v8::Object::New(isolate);
		v8::Local<v8::Map> v_headers = v8::Map::New(isolate);
		for (auto const& field : *res) {
			string key = boost::beast::http::to_string(field.name()).to_string();
			string val = field.value().to_string();
			err = StrToV8(v_key, key, isolate);
			if (err != "") {
				RejectErr(err);
				return;
			}
			err = StrToV8(v_val, val, isolate);
			if (err != "") {
				RejectErr(err + " (" + url + ")");
				return;
			}
			if (v_headers->Set(context, v_key, v_val).IsEmpty()) {
				RejectErr("Can't set headers value (" + url + "): " + key + " => " + val);
				return;
			}
		}
	//url
		err = StrToV8(v_key, "url", isolate);
		if (err != "") {
			RejectErr(err + " (" + url + ")");
			return;
		}
		err = StrToV8(v_val, url, isolate);
		if (err != "") {
			RejectErr(err);
			return;
		}
		;
		if (!v_req->Set(context, v_key, v_val).FromMaybe(false)) {
			RejectErr("Can't set url (" + url + ")");
			return;
		}
	//status
		auto status = res->result_int();
		err = StrToV8(v_key, "status", isolate);
		if (err != "") {
			RejectErr(err);
			return;
		}
		if (!v_req->Set(context, v_key, v8::Integer::New(isolate, status)).FromMaybe(false)) {
			RejectErr("Can't set status (" + url + ")");
			return;
		}
	//headers
		err = StrToV8(v_key, "headers", isolate);
		if (err != "") {
			RejectErr(err + " (" + url + ")");
			return;
		}
		if (!v_req->Set(context, v_key, v_headers).FromMaybe(false)) {
			RejectErr("Can't set headers (" + url + ")");
			return;
		}
	//body
		err = StrToV8(v_key, "body", isolate);
		if (err != "") {
			RejectErr(err + " (" + url + ")");
			return;
		}
		err = StrToV8(v_val, res->body(), isolate);
		if (err != "") {
			RejectErr("Can't create body v8 string (" + url + ")");
			return;
		}
		if (!v_req->Set(context, v_key, v_val).FromMaybe(false)) {
			RejectErr("Can't set headers (" + url + ")");
			return;
		}

		if (!resolver->Resolve(context, v_req).FromMaybe(false)) {
			RejectErr("Can't resolve promise (" + url + ")");
		}

/*!!!
		v8::Local<v8::Function> func = fetch_args->cb.Get(isolate);
		v8::Local<v8::Value> params[6];

		v8::Local<v8::Value> result;
		if (!func->Call(context, context->Global(), 6, params).ToLocal(&result)) {
//		if (!func->Call(context, fthis, 5, params).ToLocal(&result)) {
			string ex = myv8::GetException(context, try_catch);
			if (ex != "") {
				cout << ">>" << ex << "\n";
				return;
			}
			cout << mylog::GetErr("fetch>>Can't call fetch callback", string(__FUNCTION__)) << "\n";
		}*/
	});
}
}

}
