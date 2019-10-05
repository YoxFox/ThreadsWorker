#ifndef __BASEBLOCKWORKER_H__
#define __BASEBLOCKWORKER_H__

#include <atomic>

#include "../../types/lrdatabuffer.h"
#include "../../interfaces/iworker.h"

namespace twPro {

    class BaseBlockWorker : public twPro::IWorker
    {
    public:

        BaseBlockWorker() noexcept;
        virtual ~BaseBlockWorker() noexcept;

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different trhreads many times.
        // Stop flag: TRUE is STOP, FALSE is continue
        twPro::Result<twPro::WORKER_CODES> work(std::atomic_bool & _stopFlag) noexcept override;

        // Input data buffer for consuming
        void setConsumerBuffer(const std::shared_ptr<twPro::LRDataBuffer> & _cBuffer) noexcept;

        // Output data buffer for producing
        void setProducerBuffer(const std::shared_ptr<twPro::LRDataBuffer> & _pBuffer) noexcept;

    protected:

        virtual twPro::Result<twPro::WORKER_CODES> doBlockWork(const std::shared_ptr<const twPro::DataUnit> & _dataUnit, const std::shared_ptr<twPro::DataUnit> & _resultUnit) const noexcept = 0;

    private:

        std::atomic_bool m_isStopped;

        std::shared_ptr<twPro::LRDataBuffer> m_dataSource;
        std::shared_ptr<twPro::LRDataBuffer> m_resultStorage;

    };

}

#endif // __BASEBLOCKWORKER_H__