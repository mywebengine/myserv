#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#include <string>
#include <unordered_map>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>

#include "./http/http.h"

using namespace std;

namespace myconfig {

class Config {
public:
	boost::asio::io_context ioc_;
	boost::asio::ssl::context sslc_;
	unordered_map<string, boost::asio::ip::tcp::resolver::results_type> resolve_result_by_href_;
	unordered_map<string, boost::beast::http::response<boost::beast::http::string_body>> res_by_href_;

	Config();
	void Run();
};

}

#include "./config.cc"

#endif
