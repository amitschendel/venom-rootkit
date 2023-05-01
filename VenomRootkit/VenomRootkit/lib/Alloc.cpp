#include "Alloc.h"

void* __cdecl operator new(size_t size, POOL_TYPE poolType, ULONG poolTag = POOL_TAG) noexcept(false)
{
    if (size == 0) {
        size = 1;
    }

    void* memory = ExAllocatePoolWithTag(poolType, size, poolTag);

    return memory;
}

void* operator new(size_t, void* where) noexcept(false) {
    return where;
}

void __cdecl operator delete(void* memory, POOL_TYPE, ULONG poolTag) noexcept
{
    if (memory) {
        ExFreePoolWithTag(memory, poolTag);
    }
}

void* __cdecl operator new[](size_t size, POOL_TYPE poolType, ULONG poolTag = POOL_TAG) noexcept(false)
{
    return operator new(size, poolType, poolTag);
}

void __cdecl operator delete[](void* memory, POOL_TYPE poolType, ULONG poolTag) noexcept
{
    return operator delete(memory, poolType, poolTag);
}

void __cdecl operator delete(void* ptr, size_t)
{
    ExFreePool(ptr);
}

void __cdecl operator delete(void* ptr)
{
    ExFreePool(ptr);
}

void __cdecl operator delete[](void* ptr, size_t)
{
    ExFreePool(ptr);
}

void __cdecl operator delete[](void* ptr)
{
    ExFreePool(ptr);
}