#ifndef MY_JS_H
#define MY_JS_H

#include "../http/http.h"
#include "../config.h"
#include "./v8/v8.h"

using namespace std;

namespace myjs {

string Init(char* argv[]);

class Instance {
public:
	myconfig::Config& conf_;
	myv8::V8Instance v8ins_;

	Instance(myconfig::Config& conf);
	string Init();
	string CreateModule(string mname);
	string RunScript(string js, string resource_name = "unknown", int resource_line_offset = 0, int resource_column_offset = 0);
	myhttp::Response Exec(myhttp::Request req); 
};

}

#include "./js.cc"

#endif
