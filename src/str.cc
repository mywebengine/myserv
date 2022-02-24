#include <string>
#include <vector>
#include <sstream>

#include "./str.h"

using namespace std;

namespace mystr {

int U8Len(unsigned char i) {
	if (i < 128) {
		return 1;
	}
	if (i < 224) {
		return 2;
	}
	if (i < 240) {
		return 3;
	}
	if (i < 247) {
		return 4;
	}
	return 0;
}
void Trim(string& str) {
	str.erase(0, str.find_first_not_of(" \t\r\n"));
	str.erase(str.find_last_not_of(" \t\r\n") + 1);  
}
void ReplaceAll(string& str, string_view from, string_view to) {
	for (size_t i = str.find(from); i != string::npos; i = str.find(from)) {
		str.replace(i, from.size(), to);
	}
}
vector<string> Split(const string& s, string_view delimiter) {
	vector<string> tokens;
	string token;
	istringstream token_stream(s);
	while (getline(token_stream, token, delimiter.data()[0])) {
		tokens.push_back(token);
	}
	return tokens;
}
void KebabToCamelStyle(string& str) {
//	if (str == "") {
//		return;
//	}
	vector<string> words = Split(str, "-");
	size_t words_len = words.size();
	if (words_len == 1) {
		return;
	}
	str = words[0];
	for (size_t i = 1; i < words_len; i++) {
		if (words[i] != "") {
			str.push_back(toupper(words[i][0]));
			str += words[i].substr(1);
		}
	}
}

}
