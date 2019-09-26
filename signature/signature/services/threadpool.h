#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

// This is some simple solution for thread pooling. 
// In the future we have to improve this service.

#include <functional>
#include <boost/asio/thread_pool.hpp>
#include <vector>
#include <memory>

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

        // I have some problems with boost threads on other PC, so, this is temporary solution.
        std::vector<std::shared_ptr<std::thread>> m_theradSTDPool;

    };

}

#endif // __THREADPOOL_H__