#include "md5bytehashworker.h"

#include "../../3rdparty/md5.h"

namespace twPro {

    static size_t OUTPUT_DATA_LENGTH = 16; // This worker 16 bytes of the MD5 hash value

    MD5ByteHashWorker::MD5ByteHashWorker() noexcept :
        BaseBlockWorker() 
    {
    }

    MD5ByteHashWorker::~MD5ByteHashWorker() noexcept
    {
    }

    size_t MD5ByteHashWorker::maxConsumingDataUnitSize() const noexcept
    {
        return std::numeric_limits<size_t>::max();
    }
    
    size_t MD5ByteHashWorker::maxProducingDataUnitSizeByConsumingDataUnitSize(const size_t _consumingDataUnitSize) const noexcept
    {
        /*UNUSED*/ (void)_consumingDataUnitSize;
        return OUTPUT_DATA_LENGTH;
    }

    void MD5ByteHashWorker::doBlockWork(const std::shared_ptr<const twPro::DataUnit>& _dataUnit, const std::shared_ptr<twPro::DataUnit>& _resultUnit) const
    {
        char* inputCharArray = reinterpret_cast<char*>(_dataUnit->ptr);
        char* outputCharArray = reinterpret_cast<char*>(_resultUnit->ptr);

        twPro_3rd::bzflag::MD5 hash_class(inputCharArray, _dataUnit->dataSize);
        hash_class.bytedigest(outputCharArray);
    }

}