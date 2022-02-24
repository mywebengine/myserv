#include <iostream>
#include <string>

#include "include/v8.h"

#include "../../../log.h"
#include "../v8.h"
#include "./log.h"

using namespace std;

namespace myv8 {
namespace log {

void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
	auto isolate = args.GetIsolate();
	v8::Local<v8::String> v_type;
	if (args.Length() > 0) {
		v_type = args[1].As<v8::String>();
	} else {
		string err = StrToV8(v_type, "log", isolate);
		if (err != "") {
			V8TrhowErr(isolate, string(__FUNCTION__) + ">>Can't convert string \"log\" to v8::String");
			return;
		}
	}
	Log(V8ToStr(v_type, isolate), V8ToStr(args[0].As<v8::String>(), isolate));
}
void Log(string_view type, string_view str) {
	if (type == "info") {
		cout << CYAN << "v8 info: " << str.data() << RESETN;
		return;
	}
	if (type == "warning") {
		cout << YELLOW << "v8 warning: " << str.data() << RESETN;
		return;
	}
	if (type == "error") {
		cout << RED << "v8 error: " << str.data() << RESETN;
		return;
	}
	cout << GREEN << "v8 log: " << str.data() << RESETN;
}
}

}
