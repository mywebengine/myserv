self = window = __mSelf = globalThis;
const __mSelfById = new Map(),
	__mExInSelf = new Set();
function __mInitExInSelf() {
	for (const i in __mSelf) {
		__mExInSelf.add(i);
	}
}
function __mSetSelf(curId, newId) {
	const c = new Map(),
		n = __mSelfById.get(newId);
	if (n !== undefined) {
		for (const i in self) {
			if (__mExInSelf.has(i)) {
				continue;
			}
			c.set(i, self[i]);
			if (n.has(i)) {
				continue;
			}
			delete self[i];
		}
		__mSelfById.set(curId, c);
		for (const [k, v] of n) {
			self[k] = v;
		}
	}
	globalThis = window = self;
}

function __mGetStr(args) {
	const arrLen = args.length;
	let str = "";
	for (let i = 0; i < arrLen; i++) {
//		const v = args[i];
//		val = typeof v !== "object" || v === self ? v : JSON.stringify(v);
//		str += i !== 0 ? ", " + val : val;
		str += i !== 0 ? ", " + args[i] : args[i];
	}
	return str;
}




globalThis = new Proxy(this, {
	get(a, b, c) {//.split(", ")
		__ccLog("get: " + __mGetStr([a, b]));
		return a[b];
	},
	set(a, b, c) {
		__ccLog("set: " + __mGetStr([a, b]));
		return a[b];
	}
});

function getPage(doc, req) {
	return new makePage(new Document(doc), req);
}
function makePage(docuemnt, req) {
	self = this;
	this.document = document;
	return mRnder(document.documentElement)
		.then(() => {
			document.res.body = document.toHtml();
			return document.res;
		});
}
/*
aa = 123
function a() {
//globalThis = {};
//	aa = 123
	__ccLog(aa);
}

a.call({});
__ccLog(aa);*/