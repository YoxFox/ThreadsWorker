#include "dataunit.h"

namespace twPro {

    template<typename Allocator>
    inline DataUnit_alloc<Allocator>::DataUnit_alloc(size_t _size, const Allocator & a) :
        ptr(m_allocator.allocate(_size)), size(_size), dataSize(_size), id(0), m_allocator(a)
    {
        if (!this->ptr) {
            throw bad_data_unit_alloc();
        }
    }

    template< typename Allocator>
    DataUnit_alloc<Allocator>::~DataUnit_alloc() noexcept
    {
        m_allocator.deallocate(reinterpret_cast<char*>(this->ptr), size);
    }

}