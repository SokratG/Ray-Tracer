#pragma once
#include <memory>
#include <limits>
#include <cassert>


#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))


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



template<typename T>
class LAllocator : public IAllocator<T>
{
private:
	enum Op {
		PLUS = 1,
		MINUS
	};
public:
	template<typename U>
	struct rebind {
		using other = LAllocator<U>;
	};
public:

	inline LAllocator() : currentSize(0), peakSize(0) {}
	inline ~LAllocator() = default;

	inline LAllocator(const LAllocator& other) {
		this->totalAllocSize = other.TotalAllocations();
		this->currentSize = other.CurrentAllocations();
		this->peakSize = other.PeakAllocations();
	}


	template<typename U>
	inline LAllocator(const LAllocator<U>& other) {
		this->totalAllocSize = other.TotalAllocations();
		this->currentSize = other.CurrentAllocations();
		this->peakSize = other.PeakAllocations();
	}

	virtual inline pointer allocate(size_type numObjects) override {
		update(numObjects, Op::PLUS);
		return reinterpret_cast<pointer>(operator new(sizeof(T) * numObjects));
	}

	virtual inline pointer allocate(size_type numObjects, const_void_pointer hint) override {
		return allocate(numObjects);
	}

	virtual inline void deallocate(pointer p, size_type numObjects) override {
		update(numObjects, Op::MINUS);
		operator delete(p);
	}

	template<typename U, typename... Args>
	void construct(U* p, Args&&...args)  {
		new(p) U(std::forward<Args>(args)...);
	}

	template<typename U>
	void destroy(U* p)  {
		p->~U();
	}


	inline long_size TotalAllocations() const { return this->totalAllocSize; }

	inline long_size CurrentAllocations() const { return this->currentSize; }

	inline long_size PeakAllocations() const { return this->peakSize; }

private:
	static long_size totalAllocSize;
	long_size currentSize;
	long_size peakSize;

private:
#pragma warning(push)
#pragma warning(disable : 26812)
	void update(size_type size, Op op) {
		if (op == Op::PLUS) {
			this->totalAllocSize += size;
			this->currentSize += size;
			if (this->currentSize > this->peakSize) {
				this->peakSize = this->currentSize;
			}
		}
		else
			this->currentSize -= size;
	}
#pragma warning(pop)
};



template<typename T>
typename  LAllocator<T>::long_size LAllocator<T>::totalAllocSize = 0;


template<typename T, typename U>
bool operator==(const LAllocator<T>& lhs, const LAllocator<U>& rhs) {
	return operator==(static_cast<T&>(lhs), static_cast<U&>(rhs));

}

template<typename T, typename U>
bool operator!=(const LAllocator<T>& lhs, const LAllocator<U>& rhs) {
	return operator!=(static_cast<T&>(lhs), static_cast<U&>(rhs));
}

template<typename T>
bool operator==(const LAllocator<T>& lhs, const LAllocator<T>& rhs) {
	return operator==((lhs), (rhs));

}

template<typename T>
bool operator!=(const LAllocator<T>& lhs, const LAllocator<T>& rhs) {
	return operator!=((lhs), (rhs));
}



