/*
g++ -c js.cc -o ./js.o -std=c++17 -Wall -pedantic -I/home/alex/cc/certify/include -I/home/alex/cc/v8 -I/home/alex/cc/v8/include

gcc -shared -o ./js.so ./js.o -I/home/alex/cc/certify/include -I/home/alex/cc/v8 -I/home/alex/cc/v8/include -lv8_monolith -L/home/alex/cc/v8/out.gn/x64.release.sample/obj/ -DV8_COMPRESS_POINTERS -pthread -ldl -std=c++17 -Wall -pedantic -lboost_system -lboost_thread -lssl -lcrypto

g++ ./jf.cc -I/home/alex/cc/certify/include -I/home/alex/cc/v8 -I/home/alex/cc/v8/include -lv8_monolith -L/home/alex/cc/v8/out.gn/x64.release.sample/obj/ -DV8_COMPRESS_POINTERS -pthread -ldl -std=c++17 -Wall -pedantic -lboost_system -lboost_thread -lssl -lcrypto -o _jf


-I/home/alex/cc/v8/include -lv8_monolith -L/home/alex/cc/v8/out.gn/x64.release.sample/obj/ -DV8_COMPRESS_POINTERS -pthread -ldl -std=c++17 -Wall -pedantic -lboost_system -lboost_thread -lssl -lcrypto

g++ ./jf.cc -I/home/alex/cc/certify/include -I/home/alex/cc/v8 -I/home/alex/cc/v8/include -lv8_monolith -L/home/alex/cc/v8/out.gn/x64.release.sample/obj/ -DV8_COMPRESS_POINTERS -pthread -ldl -std=c++17 -Wall -pedantic -lboost_system -lboost_thread -lssl -lcrypto -o _jf

clang-12 -std=c++2a -fprebuilt-module-path=../../mods -Xclang -emit-module-interface -c js.cc -o ../../mods/myjs.pcm -Wall -pedantic -I/home/alex/cc/certify/include -I/home/alex/cc/v8 -I/home/alex/cc/v8/include
 -L/home/alex/cc/v8/out.gn/x64.release.sample/obj/ -DV8_COMPRESS_POINTERS
 -lv8_monolith
-lstdc++ -lboost_system -lboost_thread -lssl -lcrypto

clang- -std=c++2a -c helloworld.cc -Xclang -emit-module-interface -o helloworld.pcm
clang-12 -c ./js.cc -Xclang -emit-module-interface -o js.pcm ../config.cc ../http/http.cc ../file.cc ./v8/v8.cc ./js/v8/api/fetch.cc ./js/v8/api/log.cc -I/home/alex/cc/certify/include -I/home/alex/cc/v8 -I/home/alex/cc/v8/include -L/home/alex/cc/v8/out.gn/x64.release.sample/obj/ -DV8_COMPRESS_POINTERS -lv8_monolith -std=c++2a -Wall -pedantic -lstdc++ -pthread -ldl -lboost_system -lboost_thread -lssl -lcrypto -lm

g++ -fPIC -Wall -g -c libhello.c

*/
#include <iostream>

#include "../config.h"
#include "../http/http.h"
#include "../file.h"
#include "./v8/v8.h"
#include "./js.h"

using namespace std;

namespace myjs {

string Init(char* argv[]) {
	return myv8::InitV8(argv);
}

Instance::Instance(myconfig::Config& conf) : conf_(conf), v8ins_(conf_) {
}
string Instance::Init() {
	string err = v8ins_.Init();
	if (err != "") {
		return err;
	}
	string self_js;
	err = myfile::ReadFile(self_js, "./js/self.js");
	if (err != "") {
		return err;
	}
	err = RunScript(self_js + "\n__mInitExInSelf()");
	if (err != "") {
		return err;
	}
	err = CreateModule("./js/addons.js");
	if (err != "") {
		return err;
	}
	return "";
}
string Instance::CreateModule(string mname) {
	v8::HandleScope scope(v8ins_.isolate_);
	v8::Local<v8::Context> context = v8ins_.context_.Get(v8ins_.isolate_);
	v8::Context::Scope context_scope(context);
	return v8ins_.CreateModule(context, mname);
}
string Instance::RunScript(string js, string resource_name, int resource_line_offset, int resource_column_offset) {
	v8::HandleScope scope(v8ins_.isolate_);
	v8::Local<v8::Context> context = v8ins_.context_.Get(v8ins_.isolate_);
	v8::Context::Scope context_scope(context);
	return myv8::RunScript(context, js, resource_name, resource_line_offset, resource_column_offset);
}
myhttp::Response Exec(myhttp::Request req) {
	return myhttp::Response();
}

}
