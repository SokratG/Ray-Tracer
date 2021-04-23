#pragma once
#include <memory>
#include <limits>
#include <cassert>
#include <functional>
#include <memory/memmanager.hpp>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))



template<typename T>
using alloc_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

template<class Allocator, typename T = typename Allocator::value_type, typename... Args>
alloc_unique_ptr<T> make_alloc_unique(Allocator allocator, Args&&... args)
{
	const auto _deleter = [allocator](T* ptr) mutable
	{
		allocator.destroy(ptr);
		allocator.deallocate(ptr, 1u);
	};

	void* memory_block = allocator.allocate(1u);
	if (memory_block)
	{
		T* object_block = static_cast<T*>(memory_block);
		allocator.construct(object_block, std::forward<Args>(args)...);
		return alloc_unique_ptr<T>{ object_block, custom_deleter };
	}

	return nullptr;
}



template<typename T>
class IAllocator
{
public:
	using value_type = T;

	using pointer = T*;
	using const_pointer = const T*;

	using void_pointer = void*;
	using const_void_pointer = const void*;

	using reference = T&;
	using const_reference = const T&;

	using size_type = size_t;
	using difference_ptr = std::ptrdiff_t;

	using long_size = long long;

	template<typename U>
	struct rebind {
		using other = IAllocator<U>;
	};
	
	inline IAllocator() = default;
	inline ~IAllocator() = default;

	virtual inline pointer allocate(size_type numObjects) = 0;
	virtual inline pointer allocate(size_type numObjects, const_void_pointer hint) = 0;
	virtual inline void deallocate(pointer p, size_type numObjects) = 0;
	
	
	virtual inline pointer address(reference value) const {
		return &value;
	}


	virtual inline const_pointer address(const_reference value) const {
		return &value;
	}

	virtual inline size_type max_size() const {
		return std::numeric_limits<size_type>::max() / sizeof(T);
	}

	IAllocator(const IAllocator& other) = delete;
	IAllocator& operator=(const IAllocator& other) = delete;
};



