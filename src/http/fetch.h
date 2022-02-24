#ifndef MY_HTTP_FETCH_H
#define MY_HTTP_FETCH_H

#include <string>
#include <memory>
#include <functional>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>

#include <boost/system/error_code.hpp>
#include <boost/asio/ssl/detail/openssl_types.hpp>

#include "./http.h"
#include "../config.h"

using namespace std;

namespace myhttp {

typedef void (FetchCallbackFunc)(boost::beast::http::response<boost::beast::http::string_body>* res, string_view err);

boost::asio::ssl::context InitSsl();

string Fetch(myconfig::Config& conf, Request& req, function<FetchCallbackFunc> cb);

class Https : public std::enable_shared_from_this<Https> {
public:
	myconfig::Config& conf_;
	Request req_;
	boost::asio::ip::tcp::resolver resolver_;
	boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
	boost::beast::http::request<boost::beast::http::empty_body> boost_req_;
	boost::beast::flat_buffer buffer_;// (Must persist between reads)
	boost::beast::http::response<boost::beast::http::string_body> res_;
	function<FetchCallbackFunc> cb_;

	explicit Https(myconfig::Config& conf, Request& req, function<FetchCallbackFunc> cb);
	string Run();
	void Connect(boost::asio::ip::tcp::resolver::results_type* results);
};
class Http : public std::enable_shared_from_this<Http> {
public:
	myconfig::Config& conf_;
	Request req_;
	boost::asio::ip::tcp::resolver resolver_;
	boost::beast::tcp_stream stream_;
	boost::beast::http::request<boost::beast::http::empty_body> boost_req_;
	boost::beast::flat_buffer buffer_;// (Must persist between reads)
	boost::beast::http::response<boost::beast::http::string_body> res_;
	function<FetchCallbackFunc> cb_;

	explicit Http(myconfig::Config& conf, Request& req, function<FetchCallbackFunc> cb);
	string Run();
	void Connect(boost::asio::ip::tcp::resolver::results_type* results);
};

}

#include "./fetch.cc"

#endif
