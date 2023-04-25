#pragma once

#include <ntifs.h>

#include "Alloc.h"
#include "Utils.h"
#include "UniquePtr.h"

namespace std {
	template <typename T, POOL_TYPE t_PoolType, ULONG t_PoolTag, typename... TArgs>
	UniquePtr<T, t_PoolType, t_PoolTag> make_unique(TArgs&&... Args) noexcept(false)
	{
		return UniquePtr<T, t_PoolType, t_PoolTag>(new(t_PoolType, t_PoolTag)T(std::forward<TArgs>(Args)...));
	}
}