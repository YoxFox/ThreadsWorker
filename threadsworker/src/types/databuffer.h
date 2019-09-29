#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include <memory>
#include <map>
#include <condition_variable>
#include <stdexcept>

#include "../system/constructordefines.h"
#include "dataunit.h"
#include "pcqueue.h"

namespace twPro {

    class DataBuffer final
    {
    public:

        DataBuffer(const size_t _bufferCapacity, const size_t _bufferUnitSize);
        ~DataBuffer() noexcept;

        COPY_FORBIDDEN(DataBuffer)

        // It blocks access to the data and frees all resources.
        // Attention! This function waits to clear resources.
        void clear() noexcept;

        // If it returns empty pointer, it means the DataBuffer doesn't have data now
        std::weak_ptr<twPro::DataUnit> producer_popWait(const long long _waitMilliseconds) noexcept;
        std::weak_ptr<twPro::DataUnit> consumer_popWait(const long long _waitMilliseconds) noexcept;

        void producer_push(const std::weak_ptr<twPro::DataUnit> & _unitPtr) noexcept;
        void producer_pushNotUsed(const std::weak_ptr<twPro::DataUnit> & _unitPtr) noexcept;

        void consumer_push(const std::weak_ptr<twPro::DataUnit> & _unitPtr) noexcept;
        void consumer_pushNotUsed(const std::weak_ptr<twPro::DataUnit> & _unitPtr) noexcept;

    private:

        // After clear() it is FALSE
        bool m_isAvailable;

        size_t m_bufferCapacity;
        size_t m_bufferUnitSize;

        twPro::PCQueue m_pcQueue;

        // Control MAP

        struct DataUnitInfo
        {
            bool inBuffer;

            DataUnitInfo() : inBuffer(false) {}
            DataUnitInfo(const bool _inBuffer) : inBuffer(_inBuffer) {}
        };

        std::map<std::shared_ptr<twPro::DataUnit>, twPro::DataBuffer::DataUnitInfo> m_availableUnits;

        // ----

        std::condition_variable m_cv;
        std::condition_variable m_clear_cv;
        mutable std::mutex m_pc_mutex;

    };

}

#endif // __DATABUFFER_H__