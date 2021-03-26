#pragma once
#include <stdint.h>
#include <mutex>


using byte = uint8_t;


inline int64_t next_power_of_2(int64_t val)
{
    if (val == 0)
        return val;
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;
    return val;
}

inline int32_t next_power_of_2(int32_t val)
{
    if (val == 0)
        return val;
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;
    return val;
}

inline uint64_t next_power_of_2(uint64_t val)
{
    if (val == 0)
        return val;

    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;
    return val;
}

inline uint32_t next_power_of_2(uint32_t val)
{
    if (val == 0)
        return val;
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;
    return val;
}

inline uint32_t next_power_of_2(float val)
{
    if (val > 1)
    {
        float f = (float)val;
        const uint32_t t = 1U << ((*(uint32_t*)&f >> 23) - 0x7f);
        return t << (t < val);
    }
    else
        return 1;
}


template<typename T>
class IMemoryManager
{
protected:
    std::mutex mem_mutex;
public:
    virtual void* allocate(const size_t) = 0;
    virtual void  free(void*) = 0;
};


template<typename T, uint64_t POOL_SIZE>
class MemoryManager : public IMemoryManager<T>
{
protected:
    static_assert(POOL_SIZE >= 8 && POOL_SIZE <= 4096);
public:
    MemoryManager() { freeBucketHead = nullptr; pool_size = next_power_of_2(POOL_SIZE); expand_pool(); }
    ~MemoryManager() { clean(); }

    virtual void* allocate(const size_t)
    {
#ifdef _USE_THREAD
        const std::lock_guard<std::mutex> lock(mem_mutex);
#endif
        if (freeBucketHead == nullptr)
            this->expand_pool();

        BucketList* head = freeBucketHead;
        freeBucketHead = head->next;
        return head;
    }

    virtual void  free(void* ptr_obj)
    {
#ifdef _USE_THREAD
        const std::lock_guard<std::mutex> lock(mem_mutex);
#endif
        BucketList* head = static_cast<BucketList*>(ptr_obj);
        head->next = freeBucketHead;
        freeBucketHead = head;
    }

private:
    void expand_pool()
    {
        size_t size = (sizeof(T) > sizeof(BucketList*)) ? sizeof(T) : sizeof(BucketList*);
            
        BucketList* head = reinterpret_cast<BucketList*>(new byte[size]);
        freeBucketHead = head;

        for (uint32_t i = 0; i < pool_size; i++) {
            head->next = reinterpret_cast<BucketList*>(new byte[size]);
            head = head->next;
        }

        head->next = nullptr;
    }
    void clean()
    {
        BucketList* next_ptr = freeBucketHead;
        for (; next_ptr; next_ptr = freeBucketHead) {
            freeBucketHead = freeBucketHead->next;
            delete[] next_ptr;
        }
    }
private:
    struct BucketList
    {
        BucketList *next;
    };
    BucketList* freeBucketHead;
    uint32_t pool_size = 0;
};


template<typename T, uint64_t POOL_SIZE>
class BitMapMemoryManager : public IMemoryManager<T>
{

};