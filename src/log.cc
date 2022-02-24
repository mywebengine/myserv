//#include <stdio.h>
#include <string>

#include "./log.h"

using namespace std;

namespace mylog {

string GetErr(string&& text, string_view func) {
	return GetErr(text, func);
}
string GetErr(string& text, string_view func) {
	return string(BOLDRED) + (func == "" ? text : string(func) + ">>" + text) + RESET;
}

}
