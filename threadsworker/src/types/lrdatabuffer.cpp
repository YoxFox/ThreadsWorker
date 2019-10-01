#include "lrdatabuffer.h"

namespace twPro {

    static const bool IN_BUFFER = true;

    template <class T>
    std::weak_ptr<T> expiredWeakPtr()
    {
        std::shared_ptr<T> sPtr(nullptr);
        return sPtr;
    }

    LRDataBuffer::LRDataBuffer(const size_t _bufferCapacity, const size_t _bufferUnitSize) :
        m_isAvailable(true),
        m_bufferCapacity(0)
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

            std::shared_ptr<DataUnit> unitPtr(unit);
            m_leftQueue.push(unitPtr);
            m_availableUnits.insert({ unitPtr, DataUnitInfo(IN_BUFFER) });
            ++m_bufferCapacity;
        }
    }

    LRDataBuffer::~LRDataBuffer() noexcept
    {
        clear();
    }

    void LRDataBuffer::clear() noexcept
    {
        {
            std::lock_guard<std::mutex> lock(m_LR_mutex);

            if (!m_isAvailable) {
                return;
            }

            m_isAvailable = false;
        }

        std::unique_lock<std::mutex> lk(m_LR_mutex);

        m_clear_cv.wait(lk, [this] {
            return size() == m_bufferCapacity;
        });

        m_availableUnits.clear();
        m_cv.notify_all();
    }

    std::weak_ptr<DataUnit> LRDataBuffer::left_popWait(const long long _waitMilliseconds) noexcept
    {
        {
            std::lock_guard<std::mutex> lock(m_LR_mutex);
            if (!m_isAvailable) {
                return expiredWeakPtr<DataUnit>();
            }
        }

        std::unique_lock<std::mutex> lk(m_LR_mutex);

        m_cv.wait_for(lk, std::chrono::milliseconds(_waitMilliseconds), [this] {
            return !m_isAvailable || !m_leftQueue.empty();
        });

        if (!m_isAvailable || m_leftQueue.empty()) {
            return expiredWeakPtr<DataUnit>();
        }

        std::weak_ptr<DataUnit> unit = leftPop();
        m_availableUnits.at(unit.lock()).inBuffer = false;

        return unit;
    }

    std::weak_ptr<DataUnit> LRDataBuffer::right_popWait(const long long _waitMilliseconds) noexcept
    {
        {
            std::lock_guard<std::mutex> lock(m_LR_mutex);
            if (!m_isAvailable) {
                return expiredWeakPtr<DataUnit>();
            }
        }

        std::unique_lock<std::mutex> lk(m_LR_mutex);

        m_cv.wait_for(lk, std::chrono::milliseconds(_waitMilliseconds), [this] {
            return !m_isAvailable || !m_rightQueue.empty();
        });

        if (!m_isAvailable || m_rightQueue.empty()) {
            return expiredWeakPtr<DataUnit>();
        }

        std::weak_ptr<DataUnit> unit = rightPop();
        m_availableUnits.at(unit.lock()).inBuffer = false;

        return unit;
    }

    void LRDataBuffer::left_push(const std::weak_ptr<twPro::DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_LR_mutex);

        auto it = m_availableUnits.find(_unitPtr.lock());
        if (it == m_availableUnits.end() || it->second.inBuffer) {
            return;
        }

        m_leftQueue.push(_unitPtr);

        it->second.inBuffer = true;
        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

    void LRDataBuffer::right_push(const std::weak_ptr<twPro::DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_LR_mutex);

        auto it = m_availableUnits.find(_unitPtr.lock());
        if (it == m_availableUnits.end() || it->second.inBuffer) {
            return;
        }

        m_rightQueue.push(_unitPtr);

        it->second.inBuffer = true;
        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

}
