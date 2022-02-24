#ifndef MY_FILE_H
#define MY_FILE_H

#include <string>

using namespace std;

namespace myfile {

string ReadFile(string& input, string_view url);

}

#include "./file.cc"

#endif
