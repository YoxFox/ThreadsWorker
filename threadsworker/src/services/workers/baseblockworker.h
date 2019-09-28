#ifndef __BASEBLOCKWORKER_H__
#define __BASEBLOCKWORKER_H__

#include <atomic>

#include "../../types/databuffer.h"
#include "../../interfaces/iworker.h"

namespace twPro {

    class BaseBlockWorker : public twPro::IWorker
    {
    public:

        BaseBlockWorker() noexcept;
        virtual ~BaseBlockWorker() noexcept;

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // It returns the control for thread by the stop flag or by some exception.
        // Stop flag: TRUE is STOP, FALSE is continue
        void work(std::atomic_bool & _stopFlag) override;

        // Input data buffer for consuming
        void setConsumerBuffer(const std::shared_ptr<twPro::DataBuffer> & _cBuffer) noexcept;

        // Output data buffer for producing
        void setProducerBuffer(const std::shared_ptr<twPro::DataBuffer> & _pBuffer) noexcept;

    protected:

        virtual void doBlockWork(const std::shared_ptr<const twPro::DataUnit> & _dataUnit, const std::shared_ptr<twPro::DataUnit> & _resultUnit) const = 0;

    private:

        std::atomic_bool m_isStopped;

        std::shared_ptr<DataBuffer> m_dataSource;
        std::shared_ptr<DataBuffer> m_resultStorage;

    };

}

#endif // __BASEBLOCKWORKER_H__