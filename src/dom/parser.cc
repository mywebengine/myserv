//#include <iostream>

#include <string>
#include <istream>
#include <fstream>
#include <sstream>

#include "../log.h"
#include "../str.h"
#include "./dom.h"
#include "./parser.h"

using namespace std;

namespace mydom {

bool isNotValidToName(const char s) {
	return s == '/' || s == '\\' || s == '=' || s == '"' || s == '\'' || s == '`' || s == '~' || s == ';' || s == '(' || s == ')' || s == '[' || s == ']' || s == ')' || s == '!' || s == '?' || s == '<' || s == '>';
}
bool isSpace(const char s) {
	return s == ' ' || s == '\t' || s == '\r' || s == '\n';
}

Parser::Parser() {
}
string Parser::Parse(string url) {
	file_.filename_ = url;
	ifstream r(url.data());
	if (!r) {
		return mylog::GetErr(GetErr(100, "Can't open file" + string(url)), string_view(__FUNCTION__));
	}
	reader_ = &r;
	reader_->clear();
	reader_->seekg(0);

	char s;
	if (reader_->get(s) && (unsigned char)s == 239 && reader_->get(s) && (unsigned char)s == 187 && reader_->get(s) && (unsigned char)s == 191) {
		is_bom_ = true;
	} else {
		reader_->seekg(0);
	}

	tok_ = NIL_TOK;
	file_.line_ = 1;
	file_.column_ = 1;

	PNode $fr = CreateDocumentFragment();
	ltags_.push_back($fr);
	int sl;
	while (ReadChar(s, sl)) {
		string err = ParseChar(s, sl);
		if (err != "") {
			return mylog::GetErr(err, string_view(__FUNCTION__));
		}
	}
	if (tok_ != NIL_TOK) {
		return mylog::GetErr(GetErr(101, "Unexpected end of file"), string_view(__FUNCTION__));
	}
	if (tok_ == TEXT_TOK) {
		ltags_.back()->last_child_->content_ = buf_.str();
	}
	for (PNode $i= $fr->first_child_; $i != nullptr; $i = $i->next_sibling_) {
		$i->parent_node_ = nullptr;
		doc_.nodes_.push_back($i);
		if (doc_.document_element_ == nullptr && $i->node_type_ == ELEMENT_NODE) {
			doc_.document_element_ = $i;
		}
	}
	ltags_.clear();

	reader_->close();
	return "";
}
string Parser::ParseChar(char& s, int& sl) {
//printf("tok: %d\n", tok_);
	switch (tok_) {
		case START_TOK:
			if (s == '/') {
				tok_ = END_TAG_TOK;
				buf_.str("");
				return "";
			}
			if (s == '!') {
				if (!ReadChar(s, sl)) {
					return mylog::GetErr(GetErr(20, "Unexpected end of file"), string_view(__FUNCTION__));
				}
				switch (s) {
					case '-':
						tok_ = MYBE_COMMENT_TOK;
						break;
					case '[':
						tok_ = MYBY_CDATA_TOK;
						break;
					case 'd':
					case 'D':
						tok_ = MYBE_DOCTYPE_TOK;
						break;
					default:
						return mylog::GetErr(GetErr(21, "Detected an error in element content"), string_view(__FUNCTION__));
				}
				buf_.str("");
				return "";
			}
			if (s == '?') {
				tok_ = INST_TOK;
				buf_.str("");
				return "";
			}
			if (isNotValidToName(s) || isSpace(s) || s == '-') {
				return mylog::GetErr(GetErr(22, "Start tag: error parsing, symbol `__s__` in the begin this name", s, sl), string_view(__FUNCTION__));
			}
			tok_ = TAG_TOK;
			buf_.str("");
			if (Write(s, sl)) {
				ltags_.push_back(ltags_.back()->AppendChild(CreateElement(file_, "")));
				return "";
			}
			return mylog::GetErr(GetErr(23, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case TAG_TOK:
			if (isSpace(s)) {
				ltags_.back()->node_name_ = buf_.str();
				tok_ = ATTR_TOK;
				return "";
			}
			if (s == '>') {
				ltags_.back()->node_name_ = buf_.str();
				tok_ = NIL_TOK;
				return "";
			}
			if (s == '/') {
				if (!ReadChar(s, sl)) {
					return mylog::GetErr(GetErr(30, "Unexpected end of file"), string_view(__FUNCTION__));
				}
				if (s != '>') {
					return mylog::GetErr(GetErr(31, "End empty tag: missing `>` symbol"), string_view(__FUNCTION__));
				}
				ltags_.back()->node_name_ = buf_.str();
				//close empty tag
				ltags_.pop_back();
				tok_ = NIL_TOK;
				return "";
			}
			if (isNotValidToName(s)) {
				return mylog::GetErr(GetErr(32, "Start tag: error parsing, symbol `__s__` in this name", s, sl), string_view(__FUNCTION__));
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(321, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case ATTR_TOK:
			if (s == '>') {
				tok_ = NIL_TOK;
				return "";
			}
			if (s == '/') {
				if (!ReadChar(s, sl)) {
					return mylog::GetErr(GetErr(40, "Unexpected end of file"), string_view(__FUNCTION__));
				}
				if (s != '>') {
					return mylog::GetErr(GetErr(41, "End empty tag: missing `>` symbol"), string_view(__FUNCTION__));
				}
				//close empty tag
				ltags_.pop_back();
				tok_ = NIL_TOK;
				return "";
			}
			if (isSpace(s)) {
				return "";
			}
			if (s == '-') {
				return mylog::GetErr(GetErr(42, "Attrbute name: error parsing, symbol `__s__` in the begin this name", s, sl), string_view(__FUNCTION__));
			}
			tok_ = ATTR_NAME_TOK;
			buf_.str("");
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(421, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case ATTR_NAME_TOK:
			if (isSpace(s)) {
				if (buf_.str().size() > 0) {
					tok_ = ATTR_QUOTE_TOK;
				}
				return "";
			}
			if (s == '=') {
				if (buf_.str().size() == 0) {
					return mylog::GetErr(GetErr(43, "Attribute name: error parsing, name is empty"), string_view(__FUNCTION__));
				}
				ltags_.back()->SetAttribute(buf_.str());
				tok_ = ATTR_QUOTE_TOK;
				buf_.str("");
				return "";
			}
			if (s == '>') {
				ltags_.back()->SetAttribute(buf_.str());
				tok_ = NIL_TOK;
				buf_.str("");
				return "";
			}
			if (isNotValidToName(s)) {
				return mylog::GetErr(GetErr(44, "Attrbute name: error parsing, symbol `__s__` in this name", s, sl), string_view(__FUNCTION__));
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(441, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case ATTR_QUOTE_TOK:
			if (s == '"' || s == '\'') {
				quote_ = s;
				tok_ = ATTR_VALUE_TOK;
				return "";
			}
			if (isSpace(s)) {
				return "";
			}
			ltags_.back()->SetAttribute(buf_.str());
			tok_ = ATTR_TOK;
			buf_.str("");
			ParseChar(s, sl);
//			return mylog::GetErr(GetErr(8, "Attribte value: there must be a quote in this place"), string_view(__FUNCTION__));
			return "";
		case ATTR_VALUE_TOK:
			if (s == quote_) {
				ltags_.back()->attributes_.back()->value_ = buf_.str();
				tok_ = ATTR_TOK;
				return "";
			}
			if (s == '<') {
				return mylog::GetErr(GetErr(45, "Attrbute value: error parsing, symbol `__s__`", s, sl), string_view(__FUNCTION__));
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(451, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case END_TAG_TOK:
			if (s == '>') {
				//close tag
				string beginName = ltags_.back()->node_name_,
					endName = buf_.str();
				if (beginName != endName) {
					return mylog::GetErr(GetErr(50, "End tag: opening and ending tag mismatch: " + beginName + " and " + endName), string_view(__FUNCTION__));
				}
				ltags_.pop_back();
				tok_ = NIL_TOK;
				return "";
			}
			if (isNotValidToName(s) || isSpace(s)) {
				return mylog::GetErr(GetErr(51, "Start tag: error parsing, symbol `__s__` in this name", s, sl), string_view(__FUNCTION__));
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(511, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case MYBE_COMMENT_TOK:
			if (s != '-') {
				return mylog::GetErr(GetErr(60, "Detected an error in element content"), string_view(__FUNCTION__));
			}
			tok_ = COMMENT_TOK;
			buf_.str("");
			ltags_.back()->AppendChild(CreateComment(file_, ""));
			return "";
		case COMMENT_TOK:
			if (s != '-') {
				if (!Write(s, sl)) {
					return mylog::GetErr(GetErr(611, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
				}
				return "";
			}
			if (!ReadChar(s, sl)) {
				return mylog::GetErr(GetErr(61, "Unexpected end of file"), string_view(__FUNCTION__));
			}
			if (s == '-') {
				tok_ = END_COMMENT_TOK;
				return "";
			}
			buf_.put('-');
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(612, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case END_COMMENT_TOK:
			if (s != '>') {
				return mylog::GetErr(GetErr(62, "Double hyphen within comment"), string_view(__FUNCTION__));
			}
			ltags_.back()->last_child_->content_ = buf_.str();
			tok_ = NIL_TOK;
			return "";
		case MYBY_CDATA_TOK:
		case MYBE_DOCTYPE_TOK:
			if (tok_ == MYBY_CDATA_TOK) {
				string_view abc = "CDATA[";
				size_t l = abc.size();
				for (size_t i = 0; i < l; i++) {
					if (s == abc[i]) {
						if (!ReadChar(s, sl)) {
							return mylog::GetErr(GetErr(70, "Unexpected end of file"), string_view(__FUNCTION__));
						}
					} else {
						return mylog::GetErr(GetErr(71, "Detected an error in element content, `__s__`", s, sl), string_view(__FUNCTION__));
					}
				}
				tok_ = CDATA_TOK;
				ltags_.back()->AppendChild(CreateCDATASection(file_, ""));
			} else {
				string_view abc = "OCTYPE";
				size_t l = abc.size();
				for (size_t i = 0; i < l; i++) {
					if (s == abc[i] || s == (char)tolower(abc[i])) {
						if (!ReadChar(s, sl)) {
							return mylog::GetErr(GetErr(72, "Unexpected end of file"), string_view(__FUNCTION__));
						}
					} else {
						return mylog::GetErr(GetErr(73, "Detected an error in element content, `__s__`", s, sl), string_view(__FUNCTION__));
					}
				}
				tok_ = DOCTYPE_TOK;
				ltags_.back()->AppendChild(CreateDocumentType(file_));
			}
			buf_.str("");
			return "";
		case CDATA_TOK:
			if (s == ']') {
				tok_ = MYBE_END_CDATA_TOK;
				return "";
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(741, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case MYBE_END_CDATA_TOK:
			if (s == ']') {
				tok_ = END_CDATA_TOK;
				return "";
			}
			tok_ = CDATA_TOK;
			buf_.put(']');
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(751, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case END_CDATA_TOK:
			if (s == '>') {
				ltags_.back()->last_child_->content_ = buf_.str();
				tok_ = NIL_TOK;
				return "";
			}
			tok_ = CDATA_TOK;
			buf_.put(']');
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(761, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case DOCTYPE_TOK:
			if (s == '>') {
				ltags_.back()->last_child_->content_ = buf_.str();
				tok_ = NIL_TOK;
				return "";
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(771, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case INST_TOK:
			if (isNotValidToName(s) || isSpace(s) || s == '-') {
				return mylog::GetErr(GetErr(80, "Instruction: error parsing, symbol `__s__` in the begin this name", s, sl), string_view(__FUNCTION__));
			} 
			tok_ = INST_NAME_TOK;
			buf_.str("");
			if (Write(s, sl)) {
				ltags_.back()->AppendChild(CreateProcessingInstruction(file_));
				return "";
			}
			return mylog::GetErr(GetErr(801, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case INST_NAME_TOK:
			if (isSpace(s)) {
				ltags_.back()->last_child_->node_name_ = buf_.str();
				buf_.str("");
				tok_ = INST_CONTENT_TOK;
				return "";
			}
			if (s == '?') {
				tok_ = MYBE_END_INST_TOK;
				return "";
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(811, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case INST_CONTENT_TOK:
			if (s == '?') {
				tok_ = MYBE_END_INST_TOK;
				return "";
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(821, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		case MYBE_END_INST_TOK:
			if (s == '>') {
				ltags_.back()->last_child_->content_ = buf_.str();
				tok_ = NIL_TOK;
				return "";
			}
			tok_ = INST_CONTENT_TOK;
			buf_.put('?');
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(831, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
		default:
			if (s == '<') {
				if (tok_ == TEXT_TOK) {
					ltags_.back()->last_child_->content_ = buf_.str();
				}
				tok_ = START_TOK;
				buf_.str("");
				return "";
			}
			if (tok_ != TEXT_TOK) {
				tok_ = TEXT_TOK;
				buf_.str("");
				ltags_.back()->AppendChild(CreateTextNode(file_, ""));
			}
			if (Write(s, sl)) {
				return "";
			}
			return mylog::GetErr(GetErr(901, "Can't write symbol `__s__`", s, sl), string_view(__FUNCTION__));
	}
	return "";
}
bool Parser::ReadChar(char& s, int& l) {
	if (!reader_->get(s)) {
		return false;
	}
	l = mystr::U8Len((unsigned char)s);
	if (s == '\n') {
		file_.line_++;
		file_.column_ = 0;
	} else {
		file_.column_ += 1;
	}
	return true;
}
bool Parser::Write(char& s, int len) {
	buf_.put(s);
	for (int i = 1; i < len; i++) {
		if (!reader_->get(s)) {
			return false;
		}
		buf_.put(s);
	}
	return true;
}
string Parser::GetErr(int stat, string text) {
	return "HTMLDom parser error >> " + text + ", file: " + file_.filename_ + ":" + to_string(file_.line_) + ":" + to_string(file_.column_) + " (" + to_string(stat) + ")";
}
string Parser::GetErr(int stat, string text, char s, int sl) {
	string tpl = "__s__";
	size_t i = text.find(tpl);
	if (i == string::npos) {
		return GetErr(stat, text);
	}
//	Write(ss, c, sl);
        stringstream ss;
	ss.put(s);
	char c;
	for (int i = 1; i < sl; i++) {
		if (reader_->get(c)) {
			ss.put(c);
		}
	}
	text.replace(i, tpl.size(), ss.str());
	return GetErr(stat, text);
}

}
