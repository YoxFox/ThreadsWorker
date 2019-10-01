#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include <memory>
#include <map>
#include <condition_variable>
#include <stdexcept>
#include <algorithm>

#include "../system/constructordefines.h"
#include "dataunit.h"

/*
    Thread-safe Left-Right data buffer.
    This buffer has 2 queue (Left and Right) for all units of the buffer.
    Every unit can be only in one of two queues or outside of the buffer.
*/

namespace twPro {

    template <class T>
    std::weak_ptr<T> expiredWeakPtr()
    {
        std::shared_ptr<T> sPtr(nullptr);
        return sPtr;
    }

    class LRDataBuffer final
    {
    public:

        LRDataBuffer(const size_t _bufferCapacity, const size_t _bufferUnitSize);
        ~LRDataBuffer() noexcept;

        COPY_FORBIDDEN(LRDataBuffer)

        // It blocks access to the data and frees all resources.
        // Attention! This function waits to clear resources.
        void clear() noexcept;

        // If it returns empty pointer, it means the DataBuffer doesn't have data now
        std::weak_ptr<twPro::DataUnit> left_popWait(const long long _waitMilliseconds) noexcept;
        std::weak_ptr<twPro::DataUnit> right_popWait(const long long _waitMilliseconds) noexcept;

        void left_push(const std::weak_ptr<twPro::DataUnit> & _unitPtr) noexcept;
        void right_push(const std::weak_ptr<twPro::DataUnit> & _unitPtr) noexcept;

    private:

        inline size_t internalUnitsNumber() { return m_leftCounter + m_rightCounter; }

        inline bool isLeftEmpty()
        {
            return m_leftCounter < 1;
        }

        inline bool isRightEmpty()
        {
            return m_rightCounter < 1;
        }

        inline std::weak_ptr<twPro::DataUnit> leftPop()
        {
            auto it = std::find_if(m_availableUnits.begin(), m_availableUnits.end(),
            [](const std::pair<std::shared_ptr<twPro::DataUnit>, twPro::LRDataBuffer::DataUnitInfo> & t) -> bool {
                return t.second.loc == twPro::LRDataBuffer::DataUnitInfo::UnitLocation::Left;
            });

            return it != m_availableUnits.end() ? it->first : expiredWeakPtr<DataUnit>();
        }

        inline std::weak_ptr<twPro::DataUnit> rightPop()
        {
            auto it = std::find_if(m_availableUnits.begin(), m_availableUnits.end(),
                [](const std::pair<std::shared_ptr<twPro::DataUnit>, twPro::LRDataBuffer::DataUnitInfo> & t) -> bool {
                return t.second.loc == twPro::LRDataBuffer::DataUnitInfo::UnitLocation::Right;
            });

            return it != m_availableUnits.end() ? it->first : expiredWeakPtr<DataUnit>();
        }

        // Control MAP

        struct DataUnitInfo
        {
            enum class UnitLocation : unsigned short
            {
                Left = 0,
                Right,
                Outside
            };

            UnitLocation loc;

            DataUnitInfo() : loc(UnitLocation::Outside) {}
            DataUnitInfo(const UnitLocation & _loc) : loc(_loc) {}
        };

        std::map<std::shared_ptr<twPro::DataUnit>, twPro::LRDataBuffer::DataUnitInfo> m_availableUnits;

        // ----

        size_t m_leftCounter;
        size_t m_rightCounter;

        // After clear() it is FALSE
        bool m_isAvailable;

        size_t m_bufferCapacity;

        std::condition_variable m_cv;
        std::condition_variable m_clear_cv;
        mutable std::mutex m_LR_mutex;

    };

}

#endif // __DATABUFFER_H__