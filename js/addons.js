import console from "./console.js";
import {Document, Element, DocumentFragment, Text, Comment} from "./document.js";
import fetch from "./fetch.js";
import fs from "./fs.js";
import FormData from "./formdata.js";
import Request from "./request.js";
import Response from "./response.js";

//globalThis.self = this;

self.console = console;

self.Document = Document;
self.Element = Element;
self.DocumentFragment = DocumentFragment;
self.Text = Text;
self.Comment = Comment;

self.fetch = fetch;
self.fs = fs;
self.FormData = FormData;
self.Request = Request;
self.Response = Response;

function getRes(__res, __doc) {
	const document = new Document(__doc),
		server = getServer(__res),
		page = getSelf(server, document);
	return mRender()
		.then(() => {
			server.res.body = document.toHtml();
			return server.res;
		});
}
function getSelf(server, document) {
	return {
		server,
		document,
        
		console,
        
		Document,
		Element,
		DocumentFragment,
		Text,
		Comment,
        
		fetch: function(a1, a2) {
			return fetch(a1, a2)
				.thne(getFetchRes);
		},
		fs,
		FormData,
		Request,
		Response,
	};
}
function getServer(res) {
	return {
		res
	};
}
function getFetchRes(res) {
}
