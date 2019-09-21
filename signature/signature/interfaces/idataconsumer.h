#pragma once

#include <atomic>
#include <memory>

#include "../types/eventhandler.h"

namespace twPro {

    class IDataConsumer
    {
    public:

        virtual ~IDataConsumer() noexcept {}

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // It returns the control for thread by the stop flag, the end of the work or some exception.
        // Stop flag: TRUE is STOP, FALSE is continue
        virtual void work(std::atomic_bool & _stopFlag) = 0;

        virtual unsigned long long currentConsumedData() const noexcept = 0;

        I_EVENT_HANDLER_MEMBER(currentConsumedData, unsigned long long)

    };

}