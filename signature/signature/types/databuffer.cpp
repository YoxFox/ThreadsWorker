#include "databuffer.h"

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
            catch (const std::exception& ex) {
                throw ex;
            }
            catch (const std::string& ex) {
                throw ex;
            }
            catch (...) {
                throw std::runtime_error("Memory allocation failed");
            }

            std::shared_ptr<DataUnit> sUnitPtr(unit);
            m_pcQueue.pPush(sUnitPtr);
            m_availableUnits.insert({ sUnitPtr, DataUnitInfo(true) });
            ++m_bufferCapacity;
        }
    }

    DataBuffer::~DataBuffer() noexcept
    {
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

        m_availableUnits.clear();
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

        std::weak_ptr<DataUnit> unit = m_pcQueue.pPop();
        m_availableUnits.at(unit.lock()).inBuffer = false;

        return unit;
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

        std::weak_ptr<DataUnit> unit = m_pcQueue.cPop();
        m_availableUnits.at(unit.lock()).inBuffer = false;

        return unit;
    }

    void DataBuffer::producer_push(const std::weak_ptr<DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pc_mutex);

        auto it = m_availableUnits.find(_unitPtr.lock());
        if (it == m_availableUnits.end() || it->second.inBuffer) {
            return;
        }

        // Push the element to the consumer queue (avaliable for consumers)
        m_pcQueue.cPush(_unitPtr);

        it->second.inBuffer = true;
        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

    void DataBuffer::producer_pushNotUsed(const std::weak_ptr<DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pc_mutex);

        auto it = m_availableUnits.find(_unitPtr.lock());
        if (it == m_availableUnits.end() || it->second.inBuffer) {
            return;
        }

        // Push the element to the producer queue (avaliable for producers)
        m_pcQueue.pPush(_unitPtr);

        it->second.inBuffer = true;
        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

    void DataBuffer::consumer_push(const std::weak_ptr<DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pc_mutex);

        auto it = m_availableUnits.find(_unitPtr.lock());
        if (it == m_availableUnits.end() || it->second.inBuffer) {
            return;
        }

        // Push the element to the producer queue (avaliable for producers)
        m_pcQueue.pPush(_unitPtr);

        it->second.inBuffer = true;
        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

    void DataBuffer::consumer_pushNotUsed(const std::weak_ptr<DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_pc_mutex);

        auto it = m_availableUnits.find(_unitPtr.lock());
        if (it == m_availableUnits.end() || it->second.inBuffer) {
            return;
        }

        // Push the element to the consumer queue (avaliable for consumers)
        m_pcQueue.cPush(_unitPtr);

        it->second.inBuffer = true;
        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

}
