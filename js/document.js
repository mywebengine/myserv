export class Document {
	constructor(doc) {
		this.documentElement = null;
		this.scrollingElement = null;
		this.head = null;
		this.body = null;		
	}
	setDocEl($e) {
		this.documentElement = $e;
		$e.parentNode = {
			nodeType: 9,
			parentNode: null
		};
		this.head = $e.getElementsByTagName("head")[0] || null;
		this.body = $e.getElementsByTagName("body")[0] || null;
	}
	createDocumentFragment() {
		return new DocumentFragment();
	}
	createElement(nodeName) {
		return new Element(nodeName, 1, new Map());
	}
	createTextNode(textContent) {
		return new Text(textContent, "#text", 3);
	}
	createComment(textContent) {
		return new Comment(textContent);
	}
	toHtml() {
		let str = "",
			$i = this.documentElement;
		if ($i === null) {
			return str;
		}
		const $parent = $i.parentNode,
			$p = [];
		do {
			if ($i.nodeType === 1) {
				str += "<" + $i.tagName;
				if ($i.attributes.size !== 0) {
					for (const [n, v] of $i.attributes) {
						str += " " + n + "=\"" + v.replaceAll("\"", "&quot;").replaceAll("'", "&apos;") + "\"";
					}
				}
				str += $i.isNotEmpty ? ">" : " />";
			} else if ($i.nodeType === 8) {
				str += "<!--" + $i.textContent + "-->";
			} else {
				str += $i.textContent;
			}
//////////////////////
			if ($i.firstChild !== null) {
				$i = $i.firstChild;
				continue;
			}
			if ($i.nodeName === "TEMPLATE") {
				$p.push($i);
				$i = $i.content.firstChild;
				continue;
			}
			if ($i.parentNode === $parent) {//если мы не ушли вглубь - значит и вправо двигаться нельзя
				break;
			}
			if ($i.nextSibling !== null) {
				if ($i.isNotEmpty) {
					str += "</" + $i.tagName + ">";
				}
				$i = $i.nextSibling;
				continue;
			}
			do {
				if ($i.isNotEmpty) {
					str += "</" + $i.tagName + ">";
				}
				$i = $i.parentNode;
				if ($i.nodeType === 11) {
					$i = $p.pop();
				}
				if ($i.parentNode === $parent) {
					$i = null;
					break;
				}
				if ($i.nextSibling !== null) {
					if ($i.isNotEmpty) {
						str += "</" + $i.tagName + ">";
					}
					$i = $i.nextSibling;
					break;
				}
			} while (true);
		} while ($i !== null);
		return str;
	}
};
export class Element {
	constructor(nodeName, nodeType, attributes, isNotEmpty = true) {//ownerDocument
		this.nodeType = nodeType;
		this.nodeName = nodeName.toUpperCase();
		this.tagName = nodeName.toLowerCase();
		this.parentNode = null;
		this.firstChild = null;
		this.lastChild = null;
		this.nextSibling = null;
		this.previousSibling = null;
		this.attributes = attributes;
		if (this.nodeName === "TEMPLATE") {
			this.content = new DocumentFragment();
		}
		this.isNotEmpty = isNotEmpty;//this.nodeName !== "LINK" && this.nodeName !== "IMG" && this.nodeName !== "BR" && this.nodeName !== "HR";
	}
	appendChild($child) {
		if ($child.nodeType === 11) {
			if ($child.firstChild === null) {
				return $child;
			}
			const $ret = $child.lastChild;
			for (let $i = $child.firstChild; $i !== null; $i = $i.nextSibling) {
				this.appendChild($i);
			}
			return $ret;
		}
		if ($child.parentNode !== null) {
			$child.parentNode.removeChild($child);
		}
		$child.parentNode = this;
		$child.nextSibling = null;
		$child.previousSibling = this.lastChild;
		if (this.firstChild === null) {
			this.firstChild = $child;
		} else {
			this.lastChild.nextSibling = $child;
		}
		this.lastChild = $child;
		return $child;
	}
	insertBefore($child, $before) {
		if ($before === null) {
			return this.appendChild($child);
		}
		for (let $i = firstChild; $i !== null; $i = $i.nextSibling) {
			if ($i !== $before) {
				continue;
			}
			if ($child.nodeType === 11) {
				if ($child.firstChild === null) {
					return $child;
				}
				for ($i = $child.firstChild; $i !== null; $i = $i.nextSibling) {
					this.insertBefore($i, $before);
				}
				return $child;
			}
			if ($child.parentNode !== null) {
				$child.parentNode.removeChild($child);
			}
			$child.parentNode = this;
			$child.nextSibling = $before;
			$child.previousSibling = $before.previousSibling;
			if (this.firstChild === $before) {
				this.firstChild = $child;
			} else {
				$before.previousSibling.nextSibling = $child;
			}
			$before.previousSibling = $child;
			return $child;
		}
		return $child;
	}
	removeChild($child) {
		for (let $i = this.firstChild; $i !== null; $i = $i.nextSibling) {
			if ($i !== $child) {
				continue;
			}
			if (this.firstChild === $child) {
				this.firstChild = $child.nextSibling;
			} else {
				$child.previousSibling.nextSibling = $child.nextSibling;
			}
			if (this.lastChild === $child) {
				this.lastChild = $child.previousSibling;
			} else {
				$child.nextSibling.previousSibling = $child.previousSibling;
			}
			$child.parentNode = null;
			$child.nextSibling = null;
			$child.previousSibling = null;
			return $child;
		}
		return $child;
	}
	replaceChild($newChild, $oldChild) {
		for (let $i = this.firstChild; $i !== null; $i = $i.nextSibling) {
			if ($i !== $oldChild) {
				continue;
			}
			$newChild.parentNode = this;
			$newChild.nextSibling = $oldChild.nextSibling;
			$newChild.previousSibling = $oldChild.previousSibling;
			if (this.firstChild === $oldChild) {
				this.firstChild = $newChild;
			}
			if (this.lastChild === $oldChild) {
				this.lastChild = $newChild;
			}
			return this.removeChild($oldChild);
		}
		return $oldChild;
	}
	cloneNode(isDeep) {
		if (this.nodeType === 1) {
			const $clone = new Element(this.nodeName, this.nodeType,  this.attributes);
			if (isDeep) {
				for (let $i = this.firstChild; $i !== null; $i = $i.nextSibling) {
					$clone.appendChild($i.cloneNode(isDeep));
				}
			}
			return $clone;
		}
		if (this.nodeType === 11) {
			const $clone = new DocumentFragment();
			for (let $i = this.firstChild; $i !== null; $i = $i.nextSibling) {
				$clone.appendChild($i.cloneNode(isDeep));
			}
			return $clone;
		}
		if (this.nodeType === 8) {
			return new Comment(this.textContent);
		}
		return new Text(this.textContent, this.nodeName, this.nodeType);
	}
	getAttribute(name) {
		const v = this.attributes.get(name);
		return v !== undefined ? v : null;
	}
	setAttribute(name, value) {
		this.attributes.set(name, value);
	}
	removeAttribute(name) {
		this.attributes.delete(name);
	}
	getElementsByTagName(findName, isFirst) {
		const res = [],
			$parent = this.parentNode,
			$p = [];
		let $i = this.firstChild;
		do {
			if ($i.nodeType === 1 && $i.tagName === findName) {
				res.push($i);
				if (isFirst) {
					return res;
				}
			}
//////////////////////
			if ($i.firstChild !== null) {
				$i = $i.firstChild;
				continue;
			}
//			if ($i.nodeName === "TEMPLATE") {
//				$p.push($i);
//				$i = $i.content.firstChild;
//				continue;
//			}
			if ($i.parentNode === $parent) {//если мы не ушли вглубь - значит и вправо двигаться нельзя
				break;
			}
			if ($i.nextSibling !== null) {
				$i = $i.nextSibling;
				continue;
			}
			do {
				$i = $i.parentNode;
//				if ($i.nodeType === 11) {
//					$i = $p.pop();
//				}
				if ($i.parentNode === $parent) {
					$i = null;
					break;
				}
				if ($i.nextSibling !== null) {
					$i = $i.nextSibling;
					break;
				}
			} while (true);
		} while ($i !== null);
		return res;
	}
}
export class DocumentFragment extends Element {
	constructor() {
		super("#document-fragment", 11, null);
	}
}
export class Text {
	constructor(textContent, nodeName, nodeType) {
		this.nodeType = nodeType;
		this.nodeName = nodeName;
		this.parentNode = null;
		this.textContent = textContent;
		this.firstChild = null;
		this.lastChild = null;
		this.nextSibling = null;
		this.previousSibling = null;
	}
}
export class Comment extends Text {
	constructor(textContent) {
		super(textContent, "#comment", 8);
	}
}
//createCDATASection
//createProcessingInstruction
