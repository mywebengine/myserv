#ifndef MY_DOM_DOM_H
#define MY_DOM_DOM_H

#include <memory>
#include <string>
#include <vector>

using namespace std;

namespace mydom {

const int ELEMENT_NODE = 1;
//const int ATTRIBUTE_NODE = 2;
const int TEXT_NODE = 3;
const int _HTML_NODE = 33;
const int CDATA_SECTION_NODE = 4;
//const int ENTITY_REFERENCE_NODE = 5;
//const int ENTITY_NODE = 6;
const int PROCESSING_INSTRUCTION_NODE = 7;
const int COMMENT_NODE = 8;
//const int DOCUMENT_NODE = 9;
const int DOCUMENT_TYPE_NODE = 10;
const int DOCUMENT_FRAGMENT_NODE = 11;
//const int NOTATION_NODE = 12;

class Node;
class Attribute;

class Document;
class Attribute;
class Node;

//typedef shared_ptr<Document> PDocument;
typedef shared_ptr<Attribute> PAttribute;
typedef shared_ptr<Node> PNode;

class Document {
public:
	Document();
	Document(int idIdx);

	Document clone();
	string ToString();

	PNode document_element_ = nullptr;
	vector<PNode> nodes_;
	int idIdx_ = 0;
};

struct File {
	string filename_;
	int line_ = 0;
	int column_ = 0;
};

class Node : public enable_shared_from_this<Node> {
public:
	Node(int node_type = 0, string_view node_name = "", string_view content = "");
	Node(File& file, int node_type = 0, string_view node_name = "", string_view content = "");

	void SetAttribute(string_view name, string_view value = "");
	void SetAttributeNode(PAttribute& attr);
//	void SetAttributeNode(PAttribute&& attr);
	void RemoveAttribute(string_view name);
	string GetAttribute(string_view name);

	const PNode& AppendChild(const PNode& $child);
	const PNode& AppendChild(const PNode&& $child);

	const PNode& InsertBefore(const PNode& $child, const PNode& $before);
	const PNode& InsertBefore(const PNode&& $child, const PNode& $before);

	const PNode& ReplaceChild(const PNode& $new_child, const PNode& $old_child);
	const PNode& ReplaceChild(const PNode&& $new_child, const PNode& $old_child);

	const PNode& RemoveChild(const PNode& $child);
	PNode CloneNode(bool isDeep);

	vector<PNode> GetElementsByTagName(string_view name);

	string ToString();

	int p_src_id_ = 0;
	int p_descr_id_ = 0;
	bool p_is_cmd_ = false;
	int p_local_id_ = 0;

	File file_;

	int node_type_ = 0;
	string node_name_;
	string content_;
	vector<PAttribute> attributes_;
//	vector<PNode> childNodes_;

	PNode parent_node_ = nullptr;
	PNode first_child_ = nullptr;
	PNode last_child_ = nullptr;
	PNode next_sibling_ = nullptr;
	PNode previous_sibling_ = nullptr;

	string QuoteText(string_view cnt);
	string QuoteComment(string_view comment);
	string QuoteCDATA(string_view data);
};

class Attribute {
public:
	Attribute(string_view name, string_view value);

	string ToString();
	string Quote(string_view val);

	string name_;
	string value_;
};

PNode CreateDocumentFragment();

PNode CreateElement(string_view name);
PNode CreateElement(File file, string_view name);

PNode CreateTextNode(string_view text);
PNode CreateTextNode(File file, string_view text);
PNode CreateHtmlNode(string_view text);

PNode CreateComment(string_view text);
PNode CreateComment(File file, string_view text);

PNode CreateCDATASection(string_view data);
PNode CreateCDATASection(File file, string_view data);

PNode CreateProcessingInstruction(string_view target = "", string_view data = "");
PNode CreateProcessingInstruction(File file, string_view target = "", string_view data = "");

PNode CreateDocumentType(string_view qualifiedNameStr = "html", string_view publicId = "", string_view systemId = "");
PNode CreateDocumentType(File file, string_view qualifiedNameStr = "html", string_view publicId = "", string_view systemId = "");

PAttribute CreateAttribute(string_view name, string_view value = "");

}

#include "./dom.cc"

#endif
