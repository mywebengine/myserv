#ifndef MY_V8_API_LOG_H
#define MY_V8_API_LOG_H

#include "include/v8.h"

#include <string>

using namespace std;

namespace myv8 {
namespace log {

void LogCallback(const v8::FunctionCallbackInfo<v8::Value>& args);
void Log(string_view type, string_view val);

}
}

#include "./log.cc"

#endif
