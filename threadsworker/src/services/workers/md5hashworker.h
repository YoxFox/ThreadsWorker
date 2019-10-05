#ifndef __MD5HASHWORKER_H__
#define __MD5HASHWORKER_H__

#include <atomic>

#include "../../system/constructordefines.h"
#include "baseblockworker.h"

namespace twPro {

    class MD5HashWorker final : public twPro::BaseBlockWorker
    {
    public:
        MD5HashWorker() noexcept;
        ~MD5HashWorker() noexcept override;

        COPY_MOVE_FORBIDDEN(MD5HashWorker)

        size_t maxConsumingDataUnitSize() const noexcept override;
        size_t maxProducingDataUnitSizeByConsumingDataUnitSize(const size_t _consumingDataUnitSize) const noexcept override;

    private:

        twPro::Result<twPro::WORKER_CODES> doBlockWork(const std::shared_ptr<const twPro::DataUnit> & _dataUnit, const std::shared_ptr<twPro::DataUnit> & _resultUnit) const noexcept override;

    };

}

#endif // __MD5HASHWORKER_H__