export default class Request {
	constructor(url, opt) {
		this.url = url;
		if (opt !== undefined) {
			this.method = opt.method || defOopt.method;
			this.headers = opt.headers || defOopt.headers;
			this.body = opt.body || defOopt.body;
			return;
		}
		this.method = defOopt.method;
		this.headers = defOopt.headers;
		this.body = defOopt.body;
	}
	getBody() {
		if (!(this.body instanceof FormData)) {
			return this.body;
		}
//'application/x-www-form-urlencoded; charset=UTF-8'
		const div = "FormBoundary" + Math.random().toString(17);
		//todo delete content-type
		this.headers.set("Content-Type", "multipart/form-data; boundary=" + div);
		let str = "------" + div;
//console.log(this.body)
		for (const [n, v] of this.body._values) {
			if (typeof v !== "object") {
				str += '\nContent-Disposition: form-data; name="' + n.replaceAll('"', "%22") + '"\n\n' + v + "\n" + div;
				continue;
			}
			const l = v.length;
			for (let i = 0; i < l; i++) {
				str += '\nContent-Disposition: form-data; name="' + n.replaceAll('"', "%22") + '"\n\n' + v[i] + "\n" + div;
			}

		}
		return str + "--";
	}
}
const defOopt  = {
	method: "get",
	headers: new Map(),
	body: ""
};
