#pragma once

#include <atomic>

namespace twPro {

    class IWorker
    {
    public:

        virtual ~IWorker() {}

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // It returns the control for thread by the stop flag or by some exception.
        // Stop flag: TRUE is STOP, FALSE is continue
        virtual void work(std::atomic_bool & _stopFlag) = 0;

    };

}