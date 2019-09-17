#include "pcqueue.h"

namespace twPro {

    PCQueue::PCQueue(const size_t capacity) noexcept
    {
    }

    PCQueue::~PCQueue() noexcept
    {
    }

    void PCQueue::pPush(const std::shared_ptr<DataUnit> & _valPtr) noexcept
    {
        m_producerQueue.push(_valPtr);
    }

    void PCQueue::cPush(const std::shared_ptr<DataUnit> & _valPtr) noexcept
    {
        m_consumerQueue.push(_valPtr);
    }

    std::shared_ptr<DataUnit> PCQueue::pPop() noexcept
    {
        if (m_producerQueue.empty()) {
            return nullptr;
        }

        std::shared_ptr<DataUnit> res = m_producerQueue.front();
        m_producerQueue.pop();

        return res;
    }

    std::shared_ptr<DataUnit> PCQueue::cPop() noexcept
    {
        if (m_consumerQueue.empty()) {
            return nullptr;
        }

        std::shared_ptr<DataUnit> res = m_consumerQueue.front();
        m_consumerQueue.pop();

        return res;
    }

    size_t PCQueue::size() const noexcept
    {
        return m_producerQueue.size() + m_consumerQueue.size();
    }

    size_t PCQueue::producerSize() const noexcept
    {
        return m_producerQueue.size();
    }

    size_t PCQueue::consumerSize() const noexcept
    {
        return m_consumerQueue.size();
    }

    bool PCQueue::producerEmpty() const noexcept
    {
        return m_producerQueue.empty();
    }

    bool PCQueue::consumerEmpty() const noexcept
    {
        return m_consumerQueue.empty();
    }

}