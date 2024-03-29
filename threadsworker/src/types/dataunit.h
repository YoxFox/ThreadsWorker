#ifndef __DATAUNIT_H__
#define __DATAUNIT_H__

#include <memory>
#include "../system/constructordefines.h"

namespace twPro {

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

        COPY_MOVE_FORBIDDEN(DataUnit_alloc)

    private:
        Allocator m_allocator;
    };

    template<typename Allocator>
    inline DataUnit_alloc<Allocator>::DataUnit_alloc(size_t _size, const Allocator & _a) :
        ptr(m_allocator.allocate(_size)), size(_size), dataSize(_size), id(0), m_allocator(_a)
    {
        if (!this->ptr) {
            throw std::runtime_error("Bad allocation memory");
        }
    }

    template< typename Allocator>
    DataUnit_alloc<Allocator>::~DataUnit_alloc() noexcept
    {
        m_allocator.deallocate(reinterpret_cast<char*>(this->ptr), size);
    }

    typedef DataUnit_alloc<> DataUnit;
}

#endif // __DATAUNIT_H__