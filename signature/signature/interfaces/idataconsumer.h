#pragma once

// REMOVE IT

/*

#include <atomic>
#include <memory>

#include "../types/eventhandler.h"
#include "../types/databuffer.h"

namespace twPro {

    class IDataConsumer
    {
    public:

        virtual ~IDataConsumer() noexcept {}

        // Input data buffer for consuming
        virtual void setConsumerBuffer(const std::shared_ptr<twPro::DataBuffer> & _buffer) noexcept = 0;

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // It returns the control for thread by the stop flag, the end of the work or some exception.
        // Stop flag: TRUE is STOP, FALSE is continue
        virtual void work(std::atomic_bool & _stopFlag) = 0;

        virtual size_t currentConsumedData() const noexcept = 0;
        virtual size_t currentConsumedDataUnits() const noexcept = 0;

        I_EVENT_HANDLER_MEMBER(currentConsumedDataUnits, size_t)

    };

}

*/