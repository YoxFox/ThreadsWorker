#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include <memory>
#include <map>
#include <condition_variable>
#include <stdexcept>
#include <queue>

#include "../system/constructordefines.h"
#include "dataunit.h"

/*
    Thread-safe Left-Right data buffer.
    This buffer has 2 queue (Left and Right) for all units of the buffer.
    Every unit can be only in one of two queues or outside of the buffer.
*/

namespace twPro {

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

        inline size_t size() { return m_leftQueue.size() + m_rightQueue.size(); }

        inline std::weak_ptr<twPro::DataUnit> leftPop()
        {
            std::weak_ptr<twPro::DataUnit> unit = m_leftQueue.front();
            m_leftQueue.pop();
            return unit;
        }

        inline std::weak_ptr<twPro::DataUnit> rightPop()
        {
            std::weak_ptr<twPro::DataUnit> unit = m_rightQueue.front();
            m_rightQueue.pop();
            return unit;
        }

        // After clear() it is FALSE
        bool m_isAvailable;

        size_t m_bufferCapacity;
        size_t m_bufferUnitSize;

        std::queue<std::weak_ptr<twPro::DataUnit>> m_leftQueue;
        std::queue<std::weak_ptr<twPro::DataUnit>> m_rightQueue;

        // Control MAP

        struct DataUnitInfo
        {
            bool inBuffer;

            DataUnitInfo() : inBuffer(false) {}
            DataUnitInfo(const bool _inBuffer) : inBuffer(_inBuffer) {}
        };

        std::map<std::shared_ptr<twPro::DataUnit>, twPro::LRDataBuffer::DataUnitInfo> m_availableUnits;

        // ----

        std::condition_variable m_cv;
        std::condition_variable m_clear_cv;
        mutable std::mutex m_LR_mutex;

    };

}

#endif // __DATABUFFER_H__