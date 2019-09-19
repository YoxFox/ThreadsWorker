#pragma once

#include <queue>
#include <memory>
#include "dataunit.h"

namespace twPro {

    class PCQueue final
    {
    public:

        PCQueue(const size_t capacity) noexcept;
        ~PCQueue() noexcept;

        /*
            p - producer queue
            c - consumer queue
        */

        /* Thread-safe */
        void pPush(const std::weak_ptr<DataUnit> & _valPtr) noexcept;
        void cPush(const std::weak_ptr<DataUnit> & _valPtr) noexcept;

        /* Thread-safe */
        std::weak_ptr<DataUnit> pPop() noexcept;
        std::weak_ptr<DataUnit> cPop() noexcept;

        /* Common size of both queues */
        size_t size() const noexcept;

        size_t producerSize() const noexcept;
        size_t consumerSize() const noexcept;

        bool producerEmpty() const noexcept;
        bool consumerEmpty() const noexcept;

    private:

        std::queue<std::weak_ptr<DataUnit>> m_producerQueue;
        std::queue<std::weak_ptr<DataUnit>> m_consumerQueue;

    };

}
