#ifndef MY_HTTP_HTTP_H
#define MY_HTTP_HTTP_H

#include <string>
#include <unordered_map>

#include "../url.h"

using namespace std;

namespace myhttp {

struct Request {
	myurl::Url url;
	string method = "get";
	unordered_map<string, string> headers;
	string body;
};
struct Response {
	unsigned int stat;
	unordered_map<string, string> headers;
	string body;
};

}

#endif
