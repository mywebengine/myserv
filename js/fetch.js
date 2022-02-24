export default function fetch(urlOrReq, opt) {
	const req = typeof urlOrReq === "string" ? new Request(urlOrReq, opt) : urlOrReq;
	return __ccFetch(req.url, req.method, req.headers, req.getBody())
		.then(res => new Response(res));
}
