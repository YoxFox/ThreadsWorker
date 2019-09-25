#pragma once

#include <atomic>

#include "../system/constructordefines.h"
#include "../../types/databuffer.h"
#include "baseblockworker.h"

namespace twPro {

    class MD5HashWorker final : public twPro::BaseBlockWorker
    {
    public:
        MD5HashWorker() noexcept;
        ~MD5HashWorker() noexcept override;

        COPY_MOVE_FORBIDDEN(MD5HashWorker)

        size_t maxConsumingDataUnitSize() const noexcept override;
        size_t maxProducingDataUnitSizeByConsumingDataUnitSize(const size_t _consumingDataUnitSize) const noexcept;

    private:

        void doBlockWork(const std::shared_ptr<const twPro::DataUnit> & _dataUnit, const std::shared_ptr<twPro::DataUnit> & _resultUnit) const override;

    };

}