#pragma once

struct DataUnit final
{
    void * const ptr;
	const size_t size;
	unsigned __int64 id;
    
	inline operator bool() { return ptr != nullptr && size > 0; }

	DataUnit(void* _ptr, size_t _size, unsigned __int64 _id) : ptr(_ptr), size(_size), id(_id) {}
	DataUnit() : ptr(nullptr), size(0), id(0) {}
	DataUnit(const DataUnit & _other) : ptr(_other.ptr), size(_other.size), id(_other.id) {}
};
