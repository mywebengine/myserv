/*
import {bb} from "./a/../b.js";
//import {cc} from "./f.js";

export function aa(str) {
//	const c = await import("./c.js");
//	bb(c.title() + str);
	bb(str);
//	log(str);
}
*/

export function pp(s) {
	self.document = new Document();


	const $d = document.createElement("html");
	$d.appendChild(document.createElement("head"));
	$d.appendChild(document.createElement("body"));

	document.setDocEl($d);

	document.body.setAttribute("style", "color: #fff");

	document.body.appendChild(document.createElement("div"));
	document.body.firstChild.insertBefore(document.createTextNode("asdasdasdas"), null);

	const $img = document.createElement("img");
	$img.setAttribute("src", "/dasdasda");
	$img.setAttribute("alt", "Alty ddsdasd");
	document.body.appendChild($img);

	const $t = document.createElement("template");
//	$t.content.appendChild($img);
	$t.content.appendChild(document.createElement("div"));
	$t.content.appendChild(document.createElement("div"));
	document.body.appendChild($t);

	console.log(document.documentElement.getElementsByTagName('div').map(i => i.nodeName));
	console.log(document.toHtml());

	fetch("https://redlabs.pw", { method: `get` })//todo method, headers body 
		.then(res => {
			console.info("REQ", JSON.stringify(Array.from(res.headers)));
			return res.text();
		})
		.then(j => {
console.info(111111111, encodeURI);
		})
		.catch(err => {
			console.error("err", err, err.stack);//err.name, err.message, err.fileName, err.lineNumber, 
		});
}
self.pp = pp;

