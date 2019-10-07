#ifndef __MD5BYTEHASHWORKER_H__
#define __MD5BYTEHASHWORKER_H__

#include <atomic>

#include "../../system/constructordefines.h"
#include "baseblockworker.h"

namespace twPro {

    class MD5ByteHashWorker final : public twPro::BaseBlockWorker
    {
    public:
        MD5ByteHashWorker() noexcept;
        ~MD5ByteHashWorker() noexcept override;

        COPY_MOVE_FORBIDDEN(MD5ByteHashWorker)

        size_t maxConsumingDataUnitSize() const noexcept override;
        size_t maxProducingDataUnitSizeByConsumingDataUnitSize(const size_t _consumingDataUnitSize) const noexcept;

    private:

        twPro::Result<twPro::WORKER_CODES> doBlockWork(const std::shared_ptr<const twPro::DataUnit> & _dataUnit, const std::shared_ptr<twPro::DataUnit> & _resultUnit) const noexcept override;

    };

}

#endif // __MD5BYTEHASHWORKER_H__