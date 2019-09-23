#pragma once

// This is some simple solution for thread pooling. 
// In the future we have to improve this service.

#include <functional>
#include <boost/asio/thread_pool.hpp>

namespace twPro {

    class ThreadPool
    {
    public:

        ThreadPool() noexcept;
        ~ThreadPool() noexcept;

        size_t countMaxAvailableThreads() const noexcept;
        void poolTask(std::function<void()> _func) noexcept;
        void join() noexcept;

    private:

        boost::asio::thread_pool m_pool;

    };

}
