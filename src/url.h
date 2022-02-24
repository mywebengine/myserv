#ifndef MY_URL_H
#define MY_URL_H

#include <string>

using namespace std;

namespace myurl {

struct Url {
    	string href;
    	string protocol;
    	string host;
    	string port;
    	string pathname;
    	string query;
};

Url Parse(string str);
void NormalizeUrl(string& url, string top_url = "");

}

#include "./url.cc"

#endif
