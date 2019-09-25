#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

// This is some simple solution for thread pooling. 
// In the future we have to improve this service.

#include <functional>
#include <boost/asio/thread_pool.hpp>

#include "../system/constructordefines.h"

namespace twPro {

    class ThreadPool
    {
    public:

        ThreadPool() noexcept;
        ~ThreadPool() noexcept;

        COPY_FORBIDDEN(ThreadPool)

        size_t countMaxAvailableThreads() const noexcept;
        void poolTask(std::function<void()> _func) noexcept;
        void join() noexcept;

    private:

        boost::asio::thread_pool m_pool;

    };

}

#endif // __THREADPOOL_H__