#include <iostream>

#include <memory>
#include <string>
#include <vector>

#include "../str.h"
#include "./dom.h"

using namespace std;

namespace mydom {

Document::Document() {
}
Document::Document(int idIdx) : idIdx_(idIdx) {
}
Document Document::clone() {
	Document newDoc(idIdx_);
	for (const PNode& $i : nodes_) {
		PNode $c = $i->CloneNode(true);
		newDoc.nodes_.push_back($c);
		if (newDoc.document_element_ == nullptr && $c->node_type_ == ELEMENT_NODE) {
			newDoc.document_element_ = $c;
		}
	}
	return newDoc;
}
string Document::ToString() {
	string str;
	for (const PNode& $i : nodes_) {
		str += $i->ToString();
	}
	return str;
}

Node::Node(int node_type, string_view node_name, string_view content) : node_type_(node_type), node_name_(node_name), content_(content) {
}
Node::Node(File& file, int node_type, string_view node_name, string_view content) : file_(file), node_type_(node_type), node_name_(node_name), content_(content) {
}
void Node::SetAttributeNode(PAttribute& attr) {
	for (PAttribute& a : attributes_) {
		if (a->name_ == attr->name_) {
			a = attr;
			return;
		}
	}
	attributes_.push_back(attr);
}
//void Node::SetAttributeNode(Attribute&& attr) {
//	  SetAttributeNode(attr);
//}
void Node::SetAttribute(string_view name, string_view value) {
	for (const PAttribute& a : attributes_) {
		if (a->name_ == name) {
			a->value_ = value;
			return;
		}
	}
	attributes_.push_back(make_shared<Attribute>(name, value));*/
}
void Node::RemoveAttribute(string_view name) {
	for (auto i = attributes_.begin(); i != attributes_.end(); ++i) {
		if ((*i)->name_ == name) {
			attributes_.erase(i);
			return;
		}
	}
}
string Node::GetAttribute(string_view name) {
	for (const PAttribute& a : attributes_) {
		if (a->name_ == name) {
			return a->value_;
		}
	}
	return "";
}
const PNode& Node::AppendChild(const PNode& $child) {
	if ($child->node_type_ == DOCUMENT_FRAGMENT_NODE) {
		if ($child->first_child_ == nullptr) {
			return $child;
		}
		PNode& $ret = $child->last_child_;
		for (PNode $i = $child->first_child_; $i != nullptr; $i = $i->next_sibling_) {
			AppendChild($i);
		}
		return $ret;
	}
	if ($child->parent_node_ != nullptr) {
		$child->parent_node_->RemoveChild($child);
	}
//cout << "->ac:" << ($child->parent_node_ != nullptr) << "==" << $child->node_type_ << $child->node_name_ << "\n";
	$child->parent_node_ = shared_from_this();
	$child->next_sibling_ = nullptr;
	$child->previous_sibling_ = last_child_;
	if (first_child_ == nullptr) {
		first_child_ = $child;
	} else {
		last_child_->next_sibling_ = $child;
	}
	last_child_ = $child;
	return $child;
}
const PNode& Node::AppendChild(const PNode&& $child) {
	return AppendChild($child);
}
const PNode& Node::InsertBefore(const PNode& $child, const PNode& $before) {
	if ($before == nullptr) {
		return AppendChild($child);
	}
	for (PNode $i = first_child_; $i != nullptr; $i = $i->next_sibling_) {
		if ($i != $before) {
			continue;
		}
		if ($child->node_type_ == DOCUMENT_FRAGMENT_NODE) {
			if ($child->first_child_ == nullptr) {
				return $child;
			}
			for ($i = $child->first_child_; $i != nullptr; $i = $i->next_sibling_) {
				InsertBefore($i, $before);
			}
			return $child;
		}
		if ($child->parent_node_ != nullptr) {
			$child->parent_node_->RemoveChild($child);
		}
		$child->parent_node_ = shared_from_this();
		$child->next_sibling_ = $before;
		$child->previous_sibling_ = $before->previous_sibling_;
		if (first_child_ == $before) {
			first_child_ = $child;
		} else {
			$before->previous_sibling_->next_sibling_ = $child;
		}
		$before->previous_sibling_ = $child;
		return $child;
	}
	return $child;
}
const PNode& Node::InsertBefore(const PNode&& $child, const PNode& $before) {
	return InsertBefore($child, $before);
}
const PNode& Node::ReplaceChild(const PNode& $new_child, const PNode& $old_child) {
	for (PNode $i = first_child_; $i != nullptr; $i = $i->next_sibling_) {
		if ($i != $old_child) {
			continue;
		}
		$new_child->parent_node_ = shared_from_this();
		$new_child->next_sibling_ = $old_child->next_sibling_;
		$new_child->previous_sibling_ = $old_child->previous_sibling_;
		if (first_child_ == $old_child) {
			first_child_ = $new_child;
		}
		if (last_child_ == $old_child) {
			last_child_ = $new_child;
		}
		return RemoveChild($old_child);
	}
	return $old_child;
}
const PNode& Node::ReplaceChild(const PNode&& $new_child, const PNode& $old_child) {
	return ReplaceChild($new_child, $old_child);
}
const PNode& Node::RemoveChild(const PNode& $child) {
	for (PNode $i = first_child_; $i != nullptr; $i = $i->next_sibling_) {
		if ($i != $child) {
			continue;
		}
		if (first_child_ == $child) {
			first_child_ = $child->next_sibling_;
		} else {
			$child->previous_sibling_->next_sibling_ = $child->next_sibling_;
		}
		if (last_child_ == $child) {
			last_child_ = $child->previous_sibling_;
		} else {
			$child->next_sibling_->previous_sibling_ = $child->previous_sibling_;
		}
		$child->parent_node_ = nullptr;
		$child->next_sibling_ = nullptr;
		$child->previous_sibling_ = nullptr;
		return $child;
	}
	return $child;
}
PNode Node::CloneNode(bool isDeep) {
	PNode $clone = make_shared<Node>(file_, node_type_, node_name_, content_);
	if (node_type_ == ELEMENT_NODE) {
		for (const PAttribute& a : attributes_) {
			$clone->SetAttribute(a->name_, a->value_);
		}
		$clone->p_src_id_ = p_src_id_;
		$clone->p_descr_id_ = p_descr_id_;
		$clone->p_is_cmd_ = p_is_cmd_;
		if (isDeep) {
			for (PNode $i = first_child_; $i != nullptr; $i = $i->next_sibling_) {
				auto $ii = $i->CloneNode(true);
				$clone->AppendChild($ii);
			}
		}
		return $clone;
	}
	if (node_type_ != DOCUMENT_FRAGMENT_NODE) {
		return $clone;
	}
	for (PNode $i = first_child_; $i != nullptr; $i = $i->next_sibling_) {
		$clone->AppendChild($i->CloneNode(isDeep));
	}
	return $clone;
}
vector<PNode> Node::GetElementsByTagName(string_view findName) {
	vector<PNode> res;
	for (PNode $i = first_child_; $i != nullptr; $i = $i->next_sibling_) {
		if ($i->node_type_ != ELEMENT_NODE) {
			continue;
		}
		if ($i->node_name_ == findName) {
			res.push_back($i);
		}
		for (const PNode& $j : $i->GetElementsByTagName(findName)) {
			res.push_back($j);
                }
	}
	return res;
}
string Node::ToString() {
	switch (node_type_) {
		case ELEMENT_NODE: {
			string str = '<' + node_name_ + " id=\"" + to_string(p_src_id_) + "/" + to_string(p_descr_id_) + "\"";
			for (const PAttribute& a : attributes_) {
				str += ' ' + a->ToString();
			}
			//to HTML
			if (node_name_ == "img" || node_name_ == "br" || node_name_ == "meta" || node_name_ == "link") {
				return str + " />";
			}
			str += '>';
			for (PNode $i = first_child_; $i != nullptr; $i = $i->next_sibling_) {
				str += $i->ToString();
			}
			return str + "</" + node_name_ + '>';
		}
		case TEXT_NODE:
			return QuoteText(content_);
		case _HTML_NODE:
			return content_;
		case COMMENT_NODE:
			return "<!--" + QuoteComment(content_) + "-->";
		case CDATA_SECTION_NODE:
			return "<![CDATA[" + QuoteCDATA(content_) + "]]>";
		case PROCESSING_INSTRUCTION_NODE:
			return "<?" + node_name_ + ' ' + content_ + "?>";
		case DOCUMENT_TYPE_NODE:
			return "<!doctype" + content_ + ">\n";
		case DOCUMENT_FRAGMENT_NODE: {
			string str;
			for (PNode $i = first_child_; $i != nullptr; $i = $i->next_sibling_) {
				str += $i->ToString();
			}
			return str;
		}
	}
	return "";
}
string Node::QuoteText(string_view text) {
	string str(text);
	mystr::ReplaceAll(str, "<", "&lt;");
	return str;
}
string Node::QuoteComment(string_view comment) {
	string str(comment);
//	mystr::ReplaceAll(str, "--", "-&#45;");
	mystr::ReplaceAll(str, "--", "-");
	return str;
}
string Node::QuoteCDATA(string_view cdata) {
	return cdata.data();
}

Attribute::Attribute(string_view name, string_view value) : name_(name), value_(value) {
}
string Attribute::ToString() {
	return name_ + "=\"" + Quote(value_) + '"';
}
string Attribute::Quote(string_view value) {
	string str(value);
	mystr::ReplaceAll(str, "\"", "&quot;");
	mystr::ReplaceAll(str, "'", "&apos;");
	return str;
}

PNode CreateDocumentFragment() {
	return make_shared<Node>(DOCUMENT_FRAGMENT_NODE, "#document-fragment");
}

PNode CreateElement(string_view name) {
	return make_shared<Node>(ELEMENT_NODE, name);
}
PNode CreateElement(File file, string_view name) {
	return make_shared<Node>(file, ELEMENT_NODE, name);
}

PNode CreateTextNode(string_view text) {
	return make_shared<Node>(TEXT_NODE, "#text", text);
}
PNode CreateTextNode(File file, string_view text) {
	return make_shared<Node>(file, TEXT_NODE, "#text", text);
}
PNode CreateHtmlNode(string_view text) {
	return make_shared<Node>(_HTML_NODE, "#html", text);
}

PNode CreateComment(string_view text) {
	return make_shared<Node>(COMMENT_NODE, "#comment", text);
}
PNode CreateComment(File file, string_view text) {
	return make_shared<Node>(file, COMMENT_NODE, "#comment", text);
}

PNode CreateCDATASection(string_view data) {
	return make_shared<Node>(CDATA_SECTION_NODE, "#CDATA", data);
}
PNode CreateCDATASection(File file, string_view data) {
	return make_shared<Node>(file, CDATA_SECTION_NODE, "#CDATA", data);
}

PNode CreateProcessingInstruction(string_view target, string_view data) {
	return make_shared<Node>(PROCESSING_INSTRUCTION_NODE, target, data);
}
PNode CreateProcessingInstruction(File file, string_view target, string_view data) {
	return make_shared<Node>(file, PROCESSING_INSTRUCTION_NODE, target, data);
}

PNode CreateDocumentType(string_view qualifiedNameStr, string_view publicId, string_view systemId) {
	return CreateDocumentType(File(), qualifiedNameStr, publicId, systemId);
}
PNode CreateDocumentType(File file, string_view qualifiedNameStr, string_view publicId, string_view systemId) {
	string cnt;
	if (qualifiedNameStr != "") {
		cnt += ' ' + string(qualifiedNameStr);
	}
	if (publicId != "") {
		cnt += ' ' + string(publicId);
	}
	if (systemId != "") {
		cnt += ' ' + string(systemId);
	}
	return make_shared<Node>(file, DOCUMENT_TYPE_NODE, "#doctype", cnt);
}

PAttribute CreateAttribute(string_view name, string_view value) {
	return make_shared<Attribute>(name, value);
}

}
