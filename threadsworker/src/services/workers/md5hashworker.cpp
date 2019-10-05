#include "md5hashworker.h"

#include "../../3rdparty/md5.h"

namespace twPro {

    static size_t OUTPUT_DATA_LENGTH = 32; // This worker creates 32 hex type values of the MD5 hash, not 16 bytes of the MD5 hash value

    MD5HashWorker::MD5HashWorker() noexcept :
        BaseBlockWorker() 
    {
    }

    MD5HashWorker::~MD5HashWorker() noexcept
    {
    }

    size_t MD5HashWorker::maxConsumingDataUnitSize() const noexcept
    {
        return std::numeric_limits<size_t>::max();
    }
    
    size_t MD5HashWorker::maxProducingDataUnitSizeByConsumingDataUnitSize(const size_t _consumingDataUnitSize) const noexcept
    {
        /*UNUSED*/ (void)_consumingDataUnitSize;
        return OUTPUT_DATA_LENGTH;
    }

    Result<WORKER_CODES> MD5HashWorker::doBlockWork(const std::shared_ptr<const twPro::DataUnit>& _dataUnit, const std::shared_ptr<twPro::DataUnit>& _resultUnit) const noexcept
    {
        char* inputCharArray = reinterpret_cast<char*>(_dataUnit->ptr);
        char* outputCharArray = reinterpret_cast<char*>(_resultUnit->ptr);

        twPro_3rd::bzflag::MD5 hash_class(inputCharArray, _dataUnit->dataSize);
        hash_class.hexdigest(outputCharArray);

        return WORKER_CODES::OK;
    }

}