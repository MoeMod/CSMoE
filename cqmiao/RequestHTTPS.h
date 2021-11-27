//
// Created by 小白白 on 2019-12-27.
//

#ifndef SDP_REQUESTHTTP_H
#define SDP_REQUESTHTTP_H

#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/utility.hpp>

#include <string>
#include <string_view>
#include <memory>

#include "root_cert.h"

namespace https {

	using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
	namespace ssl = boost::asio::ssl;       // from <boost/asio/ip/tcp.hpp>
	namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

	inline std::string append_sv(std::string in, std::string_view sv) {
		return in.append(sv.begin(), sv.end());
	}

	// Performs an HTTP GET and prints the response
	template<class CompletionHandler>
	class Session : public std::enable_shared_from_this<Session<CompletionHandler>> {
		boost::asio::io_context &ioc;
		CompletionHandler completion_handler;
		tcp::resolver resolver;
		boost::asio::ssl::context ssl_ctx{ssl::context::sslv23_client};
		//tcp::socket socket;
		ssl::stream<tcp::socket> stream;
		boost::beast::flat_buffer buf; // (Must persist between reads)
		http::request<http::string_body> req;
		http::response<http::string_body> res;
		std::ostringstream send_stream;
		std::promise<std::stringstream> output;

	public:
		// Resolver and socket require an io_context
		explicit Session(boost::asio::io_context &ioc, CompletionHandler &&completion_handler)
				: ioc(ioc), completion_handler(std::move(completion_handler)), resolver(ioc), stream(ioc, ssl_ctx)
		{
			// shared_from_this() not available yet
		}

		// Start the asynchronous operation
		void run(
				const char *host,
				const char *port,
				const char *target,
				http::verb verb) {

			// Set SNI Hostname (many hosts need this to handshake successfully)
			if(! SSL_set_tlsext_host_name(stream.native_handle(), host))
			{
				boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
				return fail(ec, "ssl");
			}

			// Set up an HTTP GET request message
			req.version(11);
			req.method(verb);
			req.target(target);
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
			req.set(http::field::accept, "*/*");
			//req.set(http::field::connection, "keep-alive");
			//req.set(http::field::accept_encoding, "gzip, deflate");
			//req.set(http::field::accept, "*/*");
			//req.set(http::field::cache_control, "no-cache");
			//req.set(http::field::content_type, "application/x-www-form-urlencoded");

			//std::cout << req << std::endl;

			// Look up the domain name
			return resolver.async_resolve(
					host,
					port,
					std::bind(
							&Session::on_resolve,
							this->shared_from_this(),
							std::placeholders::_1,
							std::placeholders::_2));
		}

		auto run(
				const char *host,
				const char *port,
				const char *target,
				http::verb verb,
				const char *content_type,
				const char *body) {
			req.set(http::field::content_type, content_type);
			req.body() = body;
			req.set(http::field::content_length, req.body().length());

			return run(host, port, target, verb);
		}

		void on_resolve(
				boost::system::error_code ec,
				tcp::resolver::results_type results) {
			if (ec)
				return fail(ec, "resolve");

			// Make the connection on the IP address we get from a lookup
			boost::asio::async_connect(
					stream.lowest_layer(),
					results.begin(),
					results.end(),
					std::bind(
							&Session::on_connect,
							this->shared_from_this(),
							std::placeholders::_1));
		}

		void on_connect(boost::system::error_code ec) {
			if (ec)
				return fail(ec, "connect");

			// Perform the SSL handshake
			stream.async_handshake(
					ssl::stream_base::client,
					std::bind(
							&Session::on_handshake,
							this->shared_from_this(),
							std::placeholders::_1));
		}

		void on_handshake(boost::system::error_code ec)
		{
			if(ec)
				return fail(ec, "handshake");

			// Send the HTTP request to the remote host
			http::async_write(stream, req,
                  std::bind(
		                  &Session::on_write,
		                  this->shared_from_this(),
		                  std::placeholders::_1,
		                  std::placeholders::_2));
		}

		void on_write(
				boost::system::error_code ec,
				std::size_t bytes_transferred) {
			boost::ignore_unused(bytes_transferred);

			if (ec)
				return fail(ec, "write");

			// Receive the HTTP response
			http::async_read(stream, buf, res,
			                 std::bind(
					                 &Session::on_read,
					                 this->shared_from_this(),
					                 std::placeholders::_1,
					                 std::placeholders::_2));
		}

		void on_read(
				boost::system::error_code ec,
				std::size_t bytes_transferred) {
			boost::ignore_unused(bytes_transferred);

			if (ec)
				return fail(ec, "read");

			// Write the message to standard out
			success();

			// Gracefully close the stream
			stream.async_shutdown(
					std::bind(
							&Session::on_shutdown,
							this->shared_from_this(),
							std::placeholders::_1
					));
		}

		void fail(boost::system::error_code ec, const std::string &what) {
			//std::cout << "HTTP Error -" << what << ": " << ec.message() << "\n";
			auto executor = boost::asio::get_associated_executor(completion_handler);
			boost::asio::dispatch(executor, [completion_handler = std::move(completion_handler), ec]() mutable
			{
				completion_handler(ec, std::string());
			});
		}

		void on_shutdown(boost::system::error_code ec)
		{
			if(ec == boost::asio::error::eof)
			{
				// Rationale:
				// http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
				ec.assign(0, ec.category());
			}

			// just ignore it
			//if(ec)
			//	return fail(ec, "shutdown");

			// If we get here then the connection is closed gracefully
		}

		void success() {
			//std::cout << res << std::endl;

			auto body = res.body();
			auto executor = boost::asio::get_associated_executor(completion_handler);
			boost::asio::dispatch(executor, [completion_handler = std::move(completion_handler), body = std::move(body)]() mutable
			{
				completion_handler(boost::system::error_code(), body);
			});
		}
	};
}

template<class CompletionToken>
auto HTTPS_RequestAsync(boost::asio::io_context &ioc, const char *method, const char *host, const char *port, const char *target, const char *content_type, const char *body, CompletionToken &&token)
{
	return boost::asio::async_initiate<CompletionToken, void(boost::system::error_code, std::string)>([&ioc, host, port, target, method, content_type, body](auto &&completion_handler){
		std::make_shared<https::Session<typename std::decay<decltype(completion_handler)>::type>>(ioc, std::forward<decltype(completion_handler)>(completion_handler))->run(host, port, target, boost::beast::http::string_to_verb(method), content_type, body);
	}, std::forward<CompletionToken>(token));
}

template<class CompletionToken>
auto HTTPS_RequestAsync(boost::asio::io_context &ioc, const char *method, const char *host, const char *port, const char *target, CompletionToken &&token)
{
	return boost::asio::async_initiate<CompletionToken, void(boost::system::error_code, std::string)>([&ioc, host, port, target, method](auto &&completion_handler){
		std::make_shared<https::Session<typename std::decay<decltype(completion_handler)>::type>>(ioc, std::forward<decltype(completion_handler)>(completion_handler))->run(host, port, target, boost::beast::http::string_to_verb(method));
	}, std::forward<CompletionToken>(token));
}


#endif //SDP_REQUESTHTTP_H