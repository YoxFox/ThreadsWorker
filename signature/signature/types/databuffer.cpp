#include "databuffer.h"

#include <iostream>

namespace twPro {

    template <class T>
    std::weak_ptr<T> expiredWeakPtr()
    {
        std::shared_ptr<T> sPtr(nullptr);
        return sPtr;
    }

    DataBuffer::DataBuffer(const size_t _bufferCapacity, const size_t _bufferUnitSize) :
        m_isAvailable(true),
        m_bufferCapacity(0), m_bufferUnitSize(_bufferUnitSize),
        m_pcQueue(_bufferCapacity)
    {
        for (auto idx = 1; idx <= _bufferCapacity; ++idx) {
            
            DataUnit* unit;

            try {
                unit = new DataUnit(_bufferUnitSize);
            }
            catch (bad_data_unit_alloc &) {
                clear();
                throw bad_data_unit_alloc();
            }

            std::shared_ptr<DataUnit> sUnitPtr(unit);
            m_pcQueue.pPush(sUnitPtr);
            m_availableUnits.push(sUnitPtr);
            ++m_bufferCapacity;
        }
    }

    DataBuffer::~DataBuffer() noexcept
    {
        std::cout << __FUNCTION__ << "\n";
        clear();
    }

    void DataBuffer::clear() noexcept
    {
        {
            std::lock_guard<std::mutex> lock(m_pc_mutex);

            if (!m_isAvailable) {
                return;
            }

            m_isAvailable = false;
        }

        std::unique_lock<std::mutex> lk(m_pc_mutex);

        m_clear_cv.wait(lk, [this] {
            return m_pcQueue.size() == m_bufferCapacity;
        });

        while (!m_availableUnits.empty()) {
            m_availableUnits.pop();
        }

        m_cv.notify_all();
    }

    std::weak_ptr<DataUnit> DataBuffer::producer_popWait(const long long _waitMilliseconds) noexcept
    {
        {
            std::lock_guard<std::mutex> lock(m_pc_mutex);
            if (!m_isAvailable) {
                return expiredWeakPtr<DataUnit>();
            }
        }

        std::unique_lock<std::mutex> lk(m_pc_mutex);

        m_cv.wait_for(lk, std::chrono::milliseconds(_waitMilliseconds), [this] {
            return !m_isAvailable || !m_pcQueue.producerEmpty();
        });

        // It's for debug: of course we can write "return m_isAvailable ? m_pcQueue.pPop() : nullptr"

        if (!m_isAvailable || m_pcQueue.producerEmpty()) {
            return expiredWeakPtr<DataUnit>();
        }

        // Take the element from the producer queue
        return m_pcQueue.pPop();
    }

    std::weak_ptr<DataUnit> DataBuffer::consumer_popWait(const long long _waitMilliseconds) noexcept
    {
        {
            std::lock_guard<std::mutex> lock(m_pc_mutex);
            if (!m_isAvailable) {
                return expiredWeakPtr<DataUnit>();
            }
        }

        std::unique_lock<std::mutex> lk(m_pc_mutex);

        m_cv.wait_for(lk, std::chrono::milliseconds(_waitMilliseconds), [this] {
            return !m_isAvailable || !m_pcQueue.consumerEmpty();
        });

        // It's for debug: of course we can write "return m_isAvailable ? m_pcQueue.cPop() : nullptr"

        if (!m_isAvailable || m_pcQueue.consumerEmpty()) {
            return expiredWeakPtr<DataUnit>();
        }

        // Take the element from the consumer queue
        return m_pcQueue.cPop();
    }

    void DataBuffer::producer_push(const std::weak_ptr<DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pc_mutex);

        // TODO: check if exsists element in the queue

        // Push the element to the consumer queue (avaliable for consumers)
        m_pcQueue.cPush(_unitPtr);

        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

    void DataBuffer::producer_pushNotUsed(const std::weak_ptr<DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pc_mutex);

        // TODO: check if exsists element in the queue

        // Push the element to the producer queue (avaliable for producers)
        m_pcQueue.pPush(_unitPtr);

        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

    void DataBuffer::consumer_push(const std::weak_ptr<DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pc_mutex);

        // TODO: check if exsists element in the queue

        // Push the element to the producer queue (avaliable for producers)
        m_pcQueue.pPush(_unitPtr);

        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

    void DataBuffer::consumer_pushNotUsed(const std::weak_ptr<DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pc_mutex);

        // TODO: check if exsists element in the queue

        // Push the element to the consumer queue (avaliable for consumers)
        m_pcQueue.cPush(_unitPtr);

        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

}