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
        void work() override;

        // Just stop the work, work() method returns the control as soon as possible. 
        // The work can be countinued by calling work() again.
        void stop() noexcept override;

    private:

        void calculateHashValue(const std::shared_ptr<const twPro::DataUnit> & _dataUnit, const std::shared_ptr<twPro::DataUnit> & _resultUnit) const;

        std::atomic_bool m_isStopped;

        std::shared_ptr<DataBuffer> m_dataProducer;
        std::shared_ptr<DataBuffer> m_resultStorage;

    };

}