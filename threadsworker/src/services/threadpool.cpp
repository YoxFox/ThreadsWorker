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
        join();
    }

    size_t ThreadPool::countMaxAvailableThreads() const noexcept
    {
        return std::thread::hardware_concurrency();
    }

    void ThreadPool::poolTask(std::function<void()> _func) noexcept
    {
        //boost::asio::post(m_pool, _func);

        std::shared_ptr<std::thread> th(new std::thread(_func));
        m_theradSTDPool.push_back(th);
    }

    void ThreadPool::join() noexcept
    {
        for (auto th : m_theradSTDPool) {
            th->join();
            th.reset();
        }

        m_theradSTDPool.clear();

        //return m_pool.join();
    }

}
