#pragma once
#include <stdint.h>
#include <mutex>
#include <forward_list>

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


inline uint32_t define_max_power_2(uint64_t size)
{
    uint32_t max_pow_2 = 0;
    for (uint64_t i = 1, p = 0; i <= size; i <<= 1, p++)
        if (i & size) {
            max_pow_2 = p;
        }
            
    return max_pow_2;
}


template<typename T>
class IMemoryManager
{
protected:
    std::mutex mem_mutex;
public:
    virtual void* allocate(const uint64_t) = 0;
    virtual void  free(void*) = 0;
};


template<typename T, uint32_t POOL_SIZE>
class OldMemoryManager : public IMemoryManager<T>
{
protected:
    static_assert(POOL_SIZE >= 1U && POOL_SIZE <= 4196U);
public:
    OldMemoryManager() {
        freeBucketHead = nullptr;
        pool_size = POOL_SIZE;
        this->expand_pool();
    }
    ~OldMemoryManager() { this->clean(); }

    virtual void* allocate([[maybe_unused]] const uint64_t size) override
    {
#ifdef _USE_THREAD
        const std::lock_guard<std::mutex> lock(mem_mutex);
#endif
        if (size == 0u)
            return nullptr;

        if (freeBucketHead == nullptr)
            this->expand_pool();

        BucketList* head = freeBucketHead;
        freeBucketHead = head->next;
        return head;
    }

    virtual void  free(void* ptr_obj) override
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
        /* align size of given type ? */
        size_t size = (sizeof(T) > sizeof(BucketList*)) ? sizeof(T) : sizeof(BucketList*);

        BucketList* head = reinterpret_cast<BucketList*>(new byte[size]);
        freeBucketHead = head;

        for (size_t i = 0; i < pool_size; i++) {
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
        BucketList* next;
    };
    BucketList* freeBucketHead;
    uint32_t pool_size = 0;
};



template<typename T, uint64_t POOL_SIZE>
class MemoryManager : public IMemoryManager<T>
{
public:
    using void_ptr = void*;
protected:
    static_assert(POOL_SIZE >= 1U && POOL_SIZE <= 4096U);
public:
    MemoryManager() { 
        freeBucketHead = nullptr; 
        pool_size = next_power_of_2(POOL_SIZE); 
        ptr_pow_2 = define_max_power_2(sizeof(BucketList*));
        expand_pool(); 
    }
    ~MemoryManager() { this->clean(); }

    virtual void_ptr allocate([[maybe_unused]] const uint64_t size) override
    {
#ifdef _USE_THREAD
        const std::lock_guard<std::mutex> lock(mem_mutex);
#endif
        if (size == 0u)
            return nullptr;

        if (freeBucketHead == nullptr)
            this->expand_pool();

        BucketList* head = freeBucketHead;
        freeBucketHead = head->next;
        return head;
    }

    virtual void  free(void_ptr ptr_obj)
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
        /* align size of given type ? */
        uint64_t size = (sizeof(T) > sizeof(BucketList*)) ? sizeof(T) : sizeof(BucketList*);

        BucketList* head = reinterpret_cast<BucketList*>(new byte[(pool_size + 1U) * size]);
        freeBucketHead = head;
        pool_list.push_front(head);

        uint64_t offset = size >> ptr_pow_2;

        for (uint32_t i = 0; i < pool_size; i++) {
            head->next = reinterpret_cast<BucketList*>(head + offset);
            head = head->next;
        }
        
        head->next = nullptr;
    }
    void clean()
    {
        for (auto& it : pool_list)
            delete[] it;
    }
private:
    struct BucketList
    {
        BucketList *next;
    };
    BucketList* freeBucketHead;
    uint32_t pool_size = 0;
    uint32_t ptr_pow_2 = 0;
    std::forward_list<BucketList*> pool_list;
};
