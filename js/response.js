export default class Response {
	constructor(res) {
		this.url = res.url;
		this.status = res.status === undefined ? 200 : res.status;
		this.ok =  this.status === 200;
		this.headers = res.headers;
		if (this.headers.contentType === undefined) {
			this.headers.contentType = "application/json";
		}
		this._body = res.body;
	}
	json() {
		return Promise.resolve(JSON.parse(this._body));
	}
	text() {
		return Promise.resolve(this._body);
	}
};
