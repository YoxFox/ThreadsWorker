#include "lrdatabuffer.h"

namespace twPro {

    LRDataBuffer::LRDataBuffer(const size_t _bufferCapacity, const size_t _bufferUnitSize) :
        m_leftCounter(0), m_rightCounter(0),
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
            m_availableUnits.insert({ unitPtr, DataUnitInfo(DataUnitInfo::UnitLocation::Left) });
            ++m_bufferCapacity;
            ++m_leftCounter;
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
            return internalUnitsNumber() == m_bufferCapacity;
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
            return !m_isAvailable || !isLeftEmpty();
        });

        if (!m_isAvailable || isLeftEmpty()) {
            return expiredWeakPtr<DataUnit>();
        }

        std::weak_ptr<DataUnit> unit = leftPop();
        m_availableUnits.at(unit.lock()).loc = DataUnitInfo::UnitLocation::Outside;
        --m_leftCounter;

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
            return !m_isAvailable || !isRightEmpty();
        });

        if (!m_isAvailable || isRightEmpty()) {
            return expiredWeakPtr<DataUnit>();
        }

        std::weak_ptr<DataUnit> unit = rightPop();
        m_availableUnits.at(unit.lock()).loc = DataUnitInfo::UnitLocation::Outside;
        --m_rightCounter;

        return unit;
    }

    void LRDataBuffer::left_push(const std::weak_ptr<twPro::DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_LR_mutex);

        auto it = m_availableUnits.find(_unitPtr.lock());
        if (it == m_availableUnits.end() || it->second.loc != DataUnitInfo::UnitLocation::Outside) {
            return;
        }

        it->second.loc = DataUnitInfo::UnitLocation::Left;

        ++m_leftCounter;
        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

    void LRDataBuffer::right_push(const std::weak_ptr<twPro::DataUnit>& _unitPtr) noexcept
    {
        std::lock_guard<std::mutex> lock(m_LR_mutex);

        auto it = m_availableUnits.find(_unitPtr.lock());
        if (it == m_availableUnits.end() || it->second.loc != DataUnitInfo::UnitLocation::Outside) {
            return;
        }

        it->second.loc = DataUnitInfo::UnitLocation::Right;

        ++m_rightCounter;
        m_cv.notify_one();
        m_clear_cv.notify_all();
    }

}
