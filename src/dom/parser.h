#ifndef MY_DOM_PARSER_H
#define MY_DOM_PARSER_H

#include <string>
#include <vector>
#include <istream>
#include <sstream>

#include "./dom.h"

using namespace std;

namespace mydom {

enum {
	NIL_TOK,
	START_TOK,

	TAG_TOK,
	ATTR_TOK,
	ATTR_NAME_TOK,
	ATTR_QUOTE_TOK,
	ATTR_VALUE_TOK,
	END_TAG_TOK,//7

	MYBE_COMMENT_TOK,
	COMMENT_TOK,
	END_COMMENT_TOK,//10

	MYBY_CDATA_TOK,
	CDATA_TOK,
	MYBE_END_CDATA_TOK,
	END_CDATA_TOK,

	INST_TOK,
	INST_NAME_TOK,
	INST_CONTENT_TOK,
	MYBE_END_INST_TOK,

	MYBE_DOCTYPE_TOK,
	DOCTYPE_TOK,//20

	TEXT_TOK
};

class Parser {
public:
	Parser();

	Document doc_;
	vector<PNode> ltags_;

	File file_;

	int tok_;
	char quote_;
	ifstream* reader_;
	stringstream buf_;
	bool is_bom_;

//private:
	string Parse(string url);

	bool ReadChar(char& s, int& len);
	string ParseChar(char& s, int& len);
	bool Write(char& s, int len);
	string GetErr(int stat, string text);
	string GetErr(int stat, string text, char s, int sl);
};

}

#include "./parser.cc"

#endif
