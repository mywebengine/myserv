#include <string>

#include "./str.h"
#include "./url.h"

using namespace std;

namespace myurl {

Url Parse(string str) {
	Url url{str};
	NormalizeUrl(url.href);
	size_t prot_idx = url.href.find("://"),
		path_idx;
	if (prot_idx == string::npos) {
		path_idx = 0;
	} else {
		url.protocol = url.href.substr(0, prot_idx);
		prot_idx += 3;
		size_t portIdx = url.href.find(':', prot_idx);
		path_idx = url.href.find('/', prot_idx);
		if (portIdx == string::npos) {
			if (url.protocol == "https") {
				url.port = "443";
			} else if (url.protocol == "http") {
				url.port = "80";
			}
			if (path_idx == string::npos) {
				url.host = url.href.substr(prot_idx);
				return url;
			}
			url.host = url.href.substr(prot_idx, path_idx - prot_idx);
		} else {
			if (path_idx == string::npos) {
				url.host = url.href.substr(prot_idx);
				return url;
			}
			url.host = url.href.substr(prot_idx, portIdx - prot_idx);
//			url.port = stoi(url.href.substr(portIdx + 1, path_idx - prot_idx));
			url.port = url.href.substr(portIdx + 1, path_idx - prot_idx);
		}
	}
	size_t q_idx = url.href.find('?', path_idx);
	if (q_idx == string::npos) {
		url.pathname = url.href.substr(path_idx);
		return url;
	}
	url.pathname = url.href.substr(path_idx, q_idx - path_idx);
	url.query = url.href.substr(q_idx);
	return url;
}
void NormalizeUrl(string& url, string top_url) {
	mystr::Trim(url);
	if (top_url != "" && url.substr(0, 1) != "/") {
		url = top_url.substr(0, top_url.find_last_of("/")) + "/" + url;
	}
	string_view protEnd = "://";
	size_t pos = url.find(protEnd);
	if (pos == string::npos) {
		pos = 0;
	} else {
		pos += 3;
	}
	while ((pos = url.find("//", pos)) != string::npos) {
		url.replace(pos, 1, "");
	}
	pos = 0;
	while ((pos = url.find("/./", pos)) != string::npos) {
		url.replace(pos, 2, "");
	}
	pos = url.size();
	while ((pos = url.rfind("../", pos)) != string::npos) {
		if (pos <= 2) {
			return;
		}
		const size_t i = url.rfind("/", pos - 2);
		if (i == string::npos) {
			break;
		}
		url.replace(i, pos + 3 - i - 1, "");
		pos = i;
	}
}

}
