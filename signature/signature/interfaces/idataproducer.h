#pragma once

#include <atomic>
#include <memory>

#include "../types/eventhandler.h"
#include "../types/databuffer.h"

namespace twPro {

    class IDataProducer
    {
    public:

        virtual ~IDataProducer() noexcept {}

        // Output data buffer for producing
        virtual void setProducerBuffer(const std::shared_ptr<twPro::DataBuffer> & _buffer) noexcept = 0;

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // It returns the control for thread by the stop flag, the end of the work or some exception.
        // Stop flag: TRUE is STOP, FALSE is continue
        virtual void work(std::atomic_bool & _stopFlag) = 0;

        // True if the data producing is done, otherwise false.
        // If true, the work() method do nothing and returns immediately.
        virtual bool isDone() const noexcept = 0;

        virtual size_t currentProducedData() const noexcept = 0;
        virtual size_t currentProducedDataUnits() const noexcept = 0;
        virtual size_t totalData() const noexcept = 0; // It returns max(return_type) for infinity data

        I_EVENT_HANDLER_MEMBER(currentProducedDataUnits, size_t)

    };

}