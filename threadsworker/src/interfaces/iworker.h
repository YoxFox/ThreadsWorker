#ifndef __IWORKER_H__
#define __IWORKER_H__

#include <atomic>
#include "../types/lrdatabuffer.h"
#include "../types/result.h"

namespace twPro {

    enum class WORKER_CODES : RESULT_CODES_TYPE
    {
        MAIN_RESULT_CODES,
        INTERNAL_ERROR
    };

    class IWorker
    {
    public:

        virtual ~IWorker() noexcept {}

        // It takes all thread time. 
        // It can be multithreadable, it can be called by different threads many times.
        // Stop flag: TRUE is STOP, FALSE is continue
        virtual twPro::Result<twPro::WORKER_CODES> work(std::atomic_bool & _stopFlag) noexcept = 0;

        // Input data buffer for consuming
        virtual void setConsumerBuffer(const std::shared_ptr<twPro::LRDataBuffer> & _cBuffer) noexcept = 0;

        // Output data buffer for producing
        virtual void setProducerBuffer(const std::shared_ptr<twPro::LRDataBuffer> & _pBuffer) noexcept = 0;

        virtual size_t maxConsumingDataUnitSize() const noexcept = 0;
        virtual size_t maxProducingDataUnitSizeByConsumingDataUnitSize(const size_t _consumingDataUnitSize) const noexcept = 0;

    };

}

#endif // __IWORKER_H__