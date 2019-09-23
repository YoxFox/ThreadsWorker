#include "threadpool.h"

#include <thread>
#include <boost/asio.hpp>

namespace twPro {

    ThreadPool::ThreadPool() noexcept :
        m_pool(std::thread::hardware_concurrency())
    {
    }

    ThreadPool::~ThreadPool() noexcept
    {
    }

    size_t ThreadPool::countMaxAvailableThreads() const noexcept
    {
        return std::thread::hardware_concurrency();
    }

    void ThreadPool::poolTask(std::function<void()> _func) noexcept
    {
        boost::asio::post(m_pool, _func);
    }

    void ThreadPool::join() noexcept
    {
        return m_pool.join();
    }

}
