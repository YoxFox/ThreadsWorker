#pragma once

#include <memory>
#include <stack>
#include <condition_variable>
#include <stdexcept>

#include "dataunit.h"
#include "pcqueue.h"

class bad_data_unit_alloc : public std::bad_alloc {
public:
    explicit bad_data_unit_alloc() : std::bad_alloc() {}
};

class DataBuffer final
{
public:

    DataBuffer(const size_t _bufferCapacity, const size_t _bufferUnitSize);
    ~DataBuffer() noexcept;

    // It blocks access to the data and frees all resources.
    // Attention! This function waits to clear resources.
    void clear() noexcept;

    // It means all units locate in the producer queue (all resources available for producers)
    bool isFullAvailable() const noexcept;
    
    // If it returns nullptr, it means the DataBuffer doesn't have data anymore
    std::shared_ptr<DataUnit> producer_popWait() noexcept;
    std::shared_ptr<const DataUnit> consumer_popWait() noexcept;

    void producer_push(const std::shared_ptr<DataUnit> & _unitPtr) noexcept;
    void consumer_push(const std::shared_ptr<const DataUnit> & _unitPtr) noexcept;

private:

    // After clear() it is FALSE
    bool m_isAvailable;

    size_t m_bufferCapacity;
    size_t m_bufferUnitSize;

    PCQueue m_pcQueue;
    std::stack<std::shared_ptr<DataUnit>> m_availableUnits;

    std::condition_variable m_cv;
    std::condition_variable m_clear_cv;
    mutable std::mutex m_pc_mutex;

};
