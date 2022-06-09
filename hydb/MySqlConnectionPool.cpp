#include "MySqlConnectionPool.h"
#include "DatabaseConfig.h"

#include <mutex>

MySqlConnectionPool::MySqlConnectionPool(boost::asio::io_context &ioc, const DatabaseConfig & c) : ioc(ioc), config(c)
{

}

MySqlConnectionPool::~MySqlConnectionPool() = default;

std::shared_ptr<boost::mysql::tcp_connection> MySqlConnectionPool::acquire()
{
    reserve(1);

    std::shared_ptr<boost::mysql::tcp_connection> ret = nullptr;
    while (ret == nullptr)
    {
        {
            std::lock_guard l(m); // 先加锁
            if (auto iter = std::find_if(v.cbegin(), v.cend(), [](const std::shared_ptr<MySqlConnection>& p) { return p->status.load() == MySqlConnection::Status::available; }); iter != v.cend())
            {
                // 有可用连接，设置后返回。
                auto conn = *iter;
                auto expected = MySqlConnection::Status::available;
                if (conn->status.compare_exchange_strong(expected, MySqlConnection::Status::in_use))
                {
                    std::shared_ptr<MySqlConnection> sp(conn.get(), [](MySqlConnection* p) {
                        assert(p->status.load() == MySqlConnection::Status::in_use);
                        p->status.store(MySqlConnection::Status::available);
                    });
                    ret = std::shared_ptr<boost::mysql::tcp_connection>(sp, &conn->connection);
                    break;
                }
            }
        }

        std::this_thread::yield();
        //continue;
    }
    return ret;
}

void MySqlConnectionPool::reserve(size_t n)
{
    if (n <= v.size())
        return;
    auto delta = n - v.size();
    std::vector<std::shared_ptr<MySqlConnection>> new_v;

    std::generate_n(std::back_inserter(new_v), delta, [&config = this->config, &ioc = this->ioc] {
        auto conn = std::make_shared<MySqlConnection>(config, ioc);
        conn->start();
        return conn;
    });

    for (auto& conn : new_v)
    {
        while (conn->status.load() != MySqlConnection::Status::available)
            std::this_thread::yield();
    }

    {
        std::lock_guard l(m);
        std::move(new_v.begin(), new_v.end(), std::back_inserter(v));
    }
}

void MySqlConnectionPool::clear()
{
    std::lock_guard l(m); // 先加锁
    v.clear();
}