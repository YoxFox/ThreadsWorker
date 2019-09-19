#pragma once

#include <memory>

namespace twPro {

    class bad_data_unit_alloc : public std::bad_alloc {
    public:
        explicit bad_data_unit_alloc() : std::bad_alloc() {}
    };

    template <typename Allocator = std::allocator<char>>
    struct DataUnit_alloc final
    {
        void * const ptr;
        const size_t size; // memory block size
        
        size_t dataSize;  // the size of data by ptr
        unsigned __int64 id;

        inline operator bool() { return ptr != nullptr && size > 0; }

        DataUnit_alloc(size_t _size, const Allocator &a = Allocator());
        ~DataUnit_alloc() noexcept;

        DataUnit_alloc(const DataUnit_alloc &) = delete;

    private:
        Allocator m_allocator;
    };

    typedef DataUnit_alloc<> DataUnit;
}