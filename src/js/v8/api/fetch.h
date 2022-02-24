#ifndef MY_V8_API_FETCH_H
#define MY_V8_API_FETCH_H

#include "include/v8.h"

#include <string>

using namespace std;

namespace myv8 {
namespace fetch {

class FetchCallbackInfo {
public:
	myhttp::Request req;
//	v8::Persistent<v8::Function> cb;
	v8::Persistent<v8::Promise::Resolver> resolver;
//	v8::Local<v8::Promise> promise;
//	v8::Persistent<v8::Context> context;

	~FetchCallbackInfo() {
//		cb.Reset();
		resolver.Reset();
//		context.Reset();
	}
};

void FetchCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
v8::Local<v8::Promise> Fetch(string req, v8::Isolate* isolate);

}
}

#include "./fetch.cc"

#endif
