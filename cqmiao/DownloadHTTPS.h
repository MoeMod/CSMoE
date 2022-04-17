#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/utility.hpp>

#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <boost/beast.hpp>

#include <string>
#include <string_view>
#include <mutex>
#include <sstream>
#include <numeric>

#include "root_cert.h"

class IDownloadSessionHandler {
public:
    virtual void on_startup(std::size_t file_size) = 0;
    virtual void on_step(std::size_t file_pos) = 0;
    virtual void on_success() = 0;
    virtual void on_failed(boost::system::error_code ec) = 0;
};

// Performs an HTTP GET and prints the response
class Session : public std::enable_shared_from_this<Session> {

protected:
    boost::asio::io_context &ioc;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ssl::context ssl_ctx{ ssl::context::sslv23_client };
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream;
    boost::beast::flat_buffer buf; // (Must persist between reads)
    boost::beast::http::request<boost::beast::http::string_body> req;
    boost::beast::http::response<boost::beast::http::string_body> res;
    boost::beast::http::response_parser<boost::beast::http::file_body> file_parser;
    std::shared_ptr<IDownloadSessionHandler> handler;
    std::size_t file_pos = 0;
    std::size_t file_size = 0;

public:
    // Resolver and socket require an io_context
    Session(boost::asio::io_context &ioc, std::shared_ptr<IDownloadSessionHandler> handler)
        : ioc(ioc), resolver(ioc), stream(ioc, ssl_ctx), handler(std::move(handler))
    {
        // shared_from_this() not available yet
    }

    // Start the asynchronous operation
    void run(
        const char* host,
        const char* port,
        const char* target,
        const char* filename
    ) 
    {

        {
            boost::system::error_code ec;
            file_parser.body_limit((std::numeric_limits<std::uint64_t>::max)());
            file_parser.get().body().open(filename, boost::beast::file_mode::write, ec);
            if(ec)
                return fail(ec, "file");
        }

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host))
        {
            boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
            return fail(ec, "ssl");
        }

        // Set up an HTTP GET request message
        req.version(11);
        req.method(boost::beast::http::verb::get);
        req.target(target);
        req.set(boost::beast::http::field::host, host);
        req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(boost::beast::http::field::accept, "*/*");

        // Look up the domain name
        resolver.async_resolve(
            host,
            port,
            std::bind(
                &Session::on_resolve,
                this->shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }

    void on_resolve(
        boost::system::error_code ec,
        boost::asio::ip::tcp::resolver::results_type results) {
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
            boost::asio::ssl::stream_base::client,
            std::bind(
                &Session::on_handshake,
                this->shared_from_this(),
                std::placeholders::_1));
    }

    void on_handshake(boost::system::error_code ec)
    {
        if (ec)
            return fail(ec, "handshake");

        // Send the HTTP request to the remote host
        boost::beast::http::async_write(stream, req,
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
        boost::beast::http::async_read_header(stream, buf, file_parser, std::bind(
            &Session::on_startup,
            this->shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
    }

    std::size_t on_startup(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        //std::cout << "on_startup: " << bytes_transferred << std::endl;
        file_size = file_parser.content_length().value_or(0);
        if (!file_size)
            ;//error
        //std::cout << "filesize: " << file_size << std::endl;
        handler->on_startup(file_size);
        boost::beast::http::async_read_some(stream, buf, file_parser, std::bind(
            &Session::on_read_some,
            this->shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
        return buf.size();
    }
    std::size_t on_read_some(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        //std::cout << "on_read_some" << std::endl;
        if (ec) {
            fail(ec, "on_read_some");
            return 0;
        }
        file_pos += bytes_transferred;
        if (file_pos >= file_size) {
            success();
            stream.async_shutdown(std::bind(&Session::on_shutdown,
                this->shared_from_this(),
                std::placeholders::_1));
            return 0;
        }
        handler->on_step(file_pos);

        //std::cout << "session::on_read_some: " << file_pos << std::endl;
        boost::beast::http::async_read_some(stream, buf, file_parser, std::bind(
            &Session::on_read_some,
            this->shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
        return buf.size();
    }

    void on_shutdown(boost::system::error_code ec)
    {
        if (ec == boost::asio::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }
        //if (ec)
        //    return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }

    void fail(boost::system::error_code ec, const std::string& what) {
        //std::cout << "HTTP Error -" << what << ": " << ec.message() << "\n";
        handler->on_failed(ec);
    }

    void success() {
        //std::cout << res << std::endl;

        auto body = res.body();

        int result_int = res.result_int();
        handler->on_success();
    }
};