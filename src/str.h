#ifndef MY_STR_H
#define MY_STR_H

#include <string>
#include <vector>

using namespace std;

namespace mystr {

int U8Len(unsigned char i);
void Trim(string& str);
void ReplaceAll(string& str, string_view from, string_view to);
vector<string> Split(const string& s, string_view delimiter);
void KebabToCamelStyle(string& str);

}

#include "./str.cc"

#endif
