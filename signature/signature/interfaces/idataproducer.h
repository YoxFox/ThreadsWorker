#pragma once

#include <atomic>

namespace twPro {

    class IDataProducer
    {
    public:

        virtual ~IDataProducer() {}

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // It returns the control for thread by the stop flag or by some exception.
        // Stop flag: TRUE is STOP, FALSE is continue
        virtual void work(std::atomic_bool & _stopFlag) = 0;

        // True if the data producing is done, otherwise false.
        // If true, the work() method do nothing and returns immediately.
        virtual bool isDone() const noexcept = 0;

        virtual unsigned long long currentProducedDataLength() const noexcept = 0;
        virtual unsigned long long totalDataLength() const noexcept = 0; // It returns max(return_type) for infinity data
    };

}