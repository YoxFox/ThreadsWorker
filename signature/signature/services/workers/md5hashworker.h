#pragma once

#include <atomic>

#include "../../types/databuffer.h"
#include "../../interfaces/iworker.h"

namespace twPro {

    class MD5HashWorker final : public twPro::IWorker
    {
    public:
        MD5HashWorker(const std::shared_ptr<DataBuffer> & _dataProducer, const std::shared_ptr<DataBuffer> & _resultStorage);
        ~MD5HashWorker();

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // It returns the control for thread by the stop flag or by some exception.
        // Stop flag: TRUE is STOP, FALSE is continue
        void work(std::atomic_bool & _stopFlag) override;

    private:

        void calculateHashValue(const std::shared_ptr<const twPro::DataUnit> & _dataUnit, const std::shared_ptr<twPro::DataUnit> & _resultUnit) const;

        std::atomic_bool m_isStopped;

        std::shared_ptr<DataBuffer> m_dataProducer;
        std::shared_ptr<DataBuffer> m_resultStorage;

    };

}