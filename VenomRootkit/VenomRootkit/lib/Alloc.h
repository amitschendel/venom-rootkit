#pragma once

#include <ntifs.h>

#include "../Config.h"

void* __cdecl operator new(size_t size, POOL_TYPE poolType, ULONG poolTag) noexcept(false);

void* operator new(size_t, void* where) noexcept(false);

void __cdecl operator delete(void* memory, POOL_TYPE poolType, ULONG poolTag) noexcept;

void* __cdecl operator new[](size_t size, POOL_TYPE poolType, ULONG poolTag) noexcept(false);

void __cdecl operator delete[](void* memory, POOL_TYPE poolType, ULONG poolTag) noexcept;

void __cdecl operator delete(void* ptr, size_t);

void __cdecl operator delete(void* ptr);

void __cdecl operator delete[](void* ptr, size_t);

void __cdecl operator delete[](void* ptr);