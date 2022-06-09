#pragma once

#include "DatabaseConfig.h"
#include <boost/mysql.hpp>
#include <boost/asio.hpp>

class MySqlConnection : public std::enable_shared_from_this<MySqlConnection>
{
public:
    MySqlConnection(DatabaseConfig config, boost::asio::io_context &io_context) :
            dbc(std::move(config)),
            conn_params(dbc.user, dbc.pass, dbc.schema, boost::mysql::collation::utf8mb4_general_ci, boost::mysql::ssl_mode::disable),
            ioc(io_context),
            resolver(ioc),
            connection(ioc)
    {

    }
    ~MySqlConnection()
    {
        boost::system::error_code ec;
        boost::mysql::error_info ei;
        connection.quit(ec, ei);
        connection.close(ec, ei);
    }

public:
    const DatabaseConfig dbc;
    boost::asio::io_context &ioc;
    boost::mysql::connection_params conn_params;  // MySQL credentials and other connection config
    boost::asio::ip::tcp::resolver resolver;
    boost::mysql::tcp_connection connection;
    boost::asio::ip::tcp::endpoint endpoint;

    enum class Status
    {
        invalid,
        failed,
        available,
        in_use,
        on_ping
    };

    void start()
    {
        resolver.async_resolve(
                dbc.host,
                dbc.port,
                std::bind(
                        &MySqlConnection::on_resolve,
                        this->shared_from_this(),
                        std::placeholders::_1,
                        std::placeholders::_2));
    }

    void on_resolve(
            boost::system::error_code ec,
            boost::asio::ip::tcp::resolver::results_type results)
    {
        if (ec)
            return fail(ec, "resolve");
        boost::asio::async_connect(connection.next_layer(),
                                   results.begin(), results.end(),
                                   std::bind(&MySqlConnection::on_connect, this->shared_from_this(), std::placeholders::_1)
        );
    }

    void on_connect(boost::system::error_code ec) {
        if (ec)
            return fail(ec, "connect");

        connection.async_handshake(conn_params,
                                   std::bind(&MySqlConnection::on_handshake, this->shared_from_this(), std::placeholders::_1)
        );
    }

    void on_handshake(const boost::system::error_code &ec) {
        if (ec)
            return fail(ec, "handshake");

        assert(status.load() == Status::invalid);
        status.store(Status::available);

        start_ping(ec);
    }

    void start_ping(const boost::system::error_code& ec)
    {
        if (ec)
            return fail(ec, "start_ping");
        using namespace std::chrono_literals;
        std::shared_ptr<boost::asio::system_timer> st = std::make_shared<boost::asio::system_timer>(ioc);
        st->expires_after(20s);
        st->async_wait([sp = shared_from_this(), st](const boost::system::error_code ec) { sp->on_ping(ec); });
    }

    void on_ping(const boost::system::error_code& ec)
    {
        if (ec)
            return fail(ec, "on_ping");
        if (auto desired = Status::available; status.compare_exchange_strong(desired, Status::on_ping))
        {
            // unique connection here
            connection.async_query("SELECT 1=1;", [sp = shared_from_this()](const boost::system::error_code &ec, boost::mysql::tcp_resultset res) {
                std::shared_ptr<boost::mysql::tcp_resultset> pres = std::make_shared<boost::mysql::tcp_resultset>(std::move(res));
                pres->async_read_all([sp, pres](const boost::system::error_code& ec, std::vector<boost::mysql::row> res) {
                    assert(sp->status.load() == Status::on_ping);
                    sp->status.store(Status::available);
                    sp->start_ping(ec);
                });
            });
        }
        else
        {
            start_ping(ec);
        }
    }

    void fail(boost::system::error_code ec, const std::string &what) {
        status.store(Status::failed);
        last_error = ec;
    }

public:
    boost::system::error_code last_error;
    std::weak_ptr<void> accessor;
    std::atomic<Status> status = Status::invalid;
};