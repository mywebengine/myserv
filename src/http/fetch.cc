#include <iostream>

#include <string>
#include <unordered_map>
#include <memory>
#include <functional>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/detail/openssl_types.hpp>

#include <boost/system/error_code.hpp>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include <boost/algorithm/string.hpp>

#include "../config.h"
#include "../log.h"
#include "./fetch.h"
#include "./http.h"

using namespace std;

namespace myhttp {

boost::asio::ssl::context InitSsl() {
	boost::asio::ssl::context sslc{boost::asio::ssl::context::tls_client};
	sslc.set_verify_mode(boost::asio::ssl::context::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
	sslc.set_default_verify_paths();
	// tag::ctx_setup_source[]
	boost::certify::enable_native_https_server_verification(sslc);
	// end::ctx_setup_source[]
	return sslc;
}

string Fetch(myconfig::Config& conf, Request& req, function<FetchCallbackFunc> cb) {
	if (req.url.protocol == "https") {
		return make_shared<Https>(conf, req, cb)->Run();
	}
	if (req.url.protocol == "http") {
		return make_shared<Http>(conf, req, cb)->Run();
	}
	return mylog::GetErr("Can't find protocol (" + req.url.protocol + ") method", string_view(__FUNCTION__));
}

void SetRequest(boost::beast::http::request<boost::beast::http::empty_body>& req_, Request& req) {
	req_.version(11);
	boost::algorithm::to_lower(req.method);
	if (req.method == "post") {
		req_.method(boost::beast::http::verb::post);
	} else if (req.method == "put") {
		req_.method(boost::beast::http::verb::put);
	} else if (req.method == "delete") {
		req_.method(boost::beast::http::verb::delete_);
	} else {
		req_.method(boost::beast::http::verb::get);
	}
//cout << "SETREQ " << req.method << req.url.host << req.url.pathname << "\n";
	if (req.url.pathname != "") {
		req_.target(req.url.pathname);
	} else {
		req_.target("/");
	}
	req_.set(boost::beast::http::field::host, req.url.host.data());
	req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
/*
accept_encoding gzip, deflate, br
authorization
cache_control,
connection
    cookie,
    cookie2,
encoding
etag
expires,
    expiry_date,
if_match,
    if_modified_since,
    if_none_match,
language,
    last_modified,
    latest_delivery_time,
referer
set_cookie,
    set_cookie2,
 www_authenticate,*/
}
boost::beast::http::response<boost::beast::http::string_body>* GetResponse(myconfig::Config& conf, Request& req, boost::beast::http::response<boost::beast::http::string_body>& res) {
	bool f = !true;
	return f ? &(conf.res_by_href_[req.url.href] = move(res)) : &res;
}

Https::Https(myconfig::Config& conf, Request& req, function<FetchCallbackFunc> cb) : conf_(conf), req_(req), resolver_(boost::asio::make_strand(conf.ioc_)), stream_(boost::asio::make_strand(conf.ioc_), conf.sslc_), cb_(cb) {
}
string Https::Run() {
//cout << "FetchHttps: " << req_.url.href << "\n";
	if (conf_.res_by_href_.count(req_.url.href) == 1) {
cout << "yeas https CACHE\n";
		cb_(&conf_.res_by_href_[req_.url.href], "");
		return "";
	}
	if (conf_.resolve_result_by_href_.count(req_.url.href) == 1) {
		Connect(&conf_.resolve_result_by_href_[req_.url.href]);
		return "";
	}
	if (!SSL_set_tlsext_host_name(stream_.native_handle(), req_.url.host.data())) {
		boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
		return mylog::GetErr(ec.message(), string_view(__FUNCTION__));
	}
	auto h = shared_from_this();
        resolver_.async_resolve(req_.url.host.data(), req_.url.port.data(), [h](boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
		if (ec) {
			h->cb_(nullptr, mylog::GetErr(ec.message(), "Https fetch resolve"));
			return;
		}
		h->Connect(&(h->conf_.resolve_result_by_href_[h->req_.url.href] = move(results)));
	});
	return "";
}
void Https::Connect(boost::asio::ip::tcp::resolver::results_type* results) {
	boost::beast::get_lowest_layer(stream_).expires_after(chrono::seconds(30));
	auto h = shared_from_this();
	boost::beast::get_lowest_layer(stream_).async_connect(*results, [h](boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type et) {
		if (ec) {
			h->cb_(nullptr, mylog::GetErr(ec.message(), "Https fetch connect"));
			return;
		}
		h->stream_.async_handshake(boost::asio::ssl::stream_base::client, [h](boost::beast::error_code ec) {
			if (ec) {
				h->cb_(nullptr, mylog::GetErr(ec.message(), "Https fetch handshake"));
				return;
			}
			SetRequest(h->boost_req_, h->req_);
			boost::beast::get_lowest_layer(h->stream_).expires_after(chrono::seconds(30));
			boost::beast::http::async_write(h->stream_, h->boost_req_, [h](boost::beast::error_code ec, size_t) {// bytesTransferred) {
//				boost::ignore_unused(bytesTransferred);
				if (ec) {
					h->cb_(nullptr, mylog::GetErr(ec.message(), "Https fetch write"));
					return;
				}
				boost::beast::http::async_read(h->stream_, h->buffer_, h->res_, [h](boost::beast::error_code ec, size_t) {// bytesTransferred) {
//				        boost::ignore_unused(bytesTransferred);
					if (ec) {
						h->cb_(nullptr, mylog::GetErr(ec.message(), "Https fetch read"));
						return;
					}
					boost::beast::get_lowest_layer(h->stream_).expires_after(chrono::seconds(30));
					h->stream_.async_shutdown([h](boost::beast::error_code ec) {
						h->stream_.next_layer().close();
						h->cb_(GetResponse(h->conf_, h->req_, h->res_), "");
					});
				});
			});
		});
	});
}

Http::Http(myconfig::Config& conf, Request& req, function<FetchCallbackFunc> cb) : conf_(conf), req_(req), resolver_(boost::asio::make_strand(conf.ioc_)), stream_(boost::asio::make_strand(conf.ioc_)), cb_(cb) {
}
string Http::Run() {
//cout << "FetchHttp: " << req_.url.href << "\n";
	if (conf_.res_by_href_.count(req_.url.href) == 1) {
cout << "yeas http CACHE\n";
		cb_(&conf_.res_by_href_[req_.url.href], "");
		return "";
	}
	if (conf_.resolve_result_by_href_.count(req_.url.href) == 1) {
		Connect(&conf_.resolve_result_by_href_[req_.url.href]);
		return "";
	}
	auto h = shared_from_this();
        resolver_.async_resolve(req_.url.host.data(), req_.url.port.data(), [h](boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
		if (ec) {
			h->cb_(nullptr, mylog::GetErr(ec.message(), "Http fetch resolve"));
			return;
		}
		h->Connect(&(h->conf_.resolve_result_by_href_[h->req_.url.href] = move(results)));
	});
	return "";
}
void Http::Connect(boost::asio::ip::tcp::resolver::results_type* results) {
	stream_.expires_after(chrono::seconds(30));
	auto h = shared_from_this();
	stream_.async_connect(*results, [h](boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type et) {
		if (ec) {
			h->cb_(nullptr, mylog::GetErr(ec.message(), "Http fetch connect"));
			return;
		}
		SetRequest(h->boost_req_, h->req_);
		h->stream_.expires_after(chrono::seconds(30));
		boost::beast::http::async_write(h->stream_, h->boost_req_, [h](boost::beast::error_code ec, size_t) {
			if (ec) {
				h->cb_(nullptr, mylog::GetErr(ec.message(), "Http fetch write"));
				return;
			}
			boost::beast::http::async_read(h->stream_, h->buffer_, h->res_, [h](boost::beast::error_code ec, size_t) {
				if (ec) {
					h->cb_(nullptr, mylog::GetErr(ec.message(), "Http fetch read"));
					return;
				}
			        h->stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
				if (ec) {
					h->cb_(nullptr, mylog::GetErr(ec.message(), "Http fetch shutdown"));
					return;
				}
				h->cb_(GetResponse(h->conf_, h->req_, h->res_), "");
			});
		});		
	});
}

}
