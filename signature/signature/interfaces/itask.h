#pragma once

#include <atomic>

namespace twPro {

    class ITask
    {
    public:
        virtual ~ITask() noexcept {}

        virtual void run(std::atomic_bool & _stopFlag) = 0;

    };

}
