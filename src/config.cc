/*
clang-12 -std=c++2a -fprebuilt-module-path=../mods -Xclang -emit-module-interface -c config.cc -o ../mods/myconfig.pcm -Wall -pedantic -I/home/alex/cc/certify/include -I/home/alex/cc/v8 -I/home/alex/cc/v8/include
*/
#include <string>
#include <unordered_map>

#include "./http/fetch.h"
#include "./config.h"

using namespace std;

namespace myconfig {

Config::Config() : sslc_(myhttp::InitSsl()) {
}
void Config::Run() {
	ioc_.run();
}

}
