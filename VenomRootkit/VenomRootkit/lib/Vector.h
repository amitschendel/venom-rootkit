#pragma once

#include <ntifs.h>

#include "Utils.h"
#include "Alloc.h"

// This class is taken from https://github.com/gauraVsehgaL/cppkernel.
template<typename T, POOL_TYPE poolType, ULONG poolTag>
class Vector
{
public:

	Vector() :m_ptr(nullptr), m_capacity(0), m_numberOfElements(0) {}

	Vector(size_t InitialNumberOfElements) :m_ptr(nullptr), m_capacity(0), m_numberOfElements(0)
	{
		reserve(InitialNumberOfElements);
		m_numberOfElements = InitialNumberOfElements;
		for (auto i = 0UL; i < m_numberOfElements; i++)
			new (m_ptr + i) T();
	}

	Vector(size_t InitialNumberOfElements, T val) :Vector(InitialNumberOfElements)
	{
		reserve(InitialNumberOfElements);
		m_numberOfElements = InitialNumberOfElements;
		for (auto i = 0UL; i < m_numberOfElements; i++)
			new (m_ptr + i) T(val);
	}

	Vector(const Vector<T, poolType, poolTag>& other) :Vector()
	{
		reserve(other.m_capacity);
		m_numberOfElements = other.m_numberOfElements;
		for (auto i = 0UL; i < m_numberOfElements; i++)
			new (m_ptr + i) T(other.m_ptr[i]);
	}

	Vector(Vector<T, poolType, poolTag>&& other) :m_capacity(other.m_capacity), m_numberOfElements(other.m_numberOfElements)
	{
		this->m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
	}

	Vector<T, poolType, poolTag>& operator=(Vector<T, poolType, poolTag>&& other)
	{
		if (this != &other)
		{
			this->m_capacity = other.m_capacity;
			this->m_numberOfElements = other.m_numberOfElements;
			this->m_ptr = other.m_ptr;
			other.m_ptr = nullptr;
		}

		return *this;
	}

	Vector<T, poolType, poolTag>& operator=(const Vector<T, poolType, poolTag>& other)
	{
		if (this != &other)
		{
			if (this->m_capacity < other.m_capacity)
			{
				this->m_capacity = other.m_capacity;
				auto OrigPtr = this->m_ptr;
				this->m_ptr = allocate(m_capacity);
				destroy(OrigPtr, this->m_numberOfElements);
				deallocate(OrigPtr);
			}

			this->m_numberOfElements = other.m_numberOfElements;

			for (auto i = 0UL; i < m_numberOfElements; i++)
				m_ptr[i] = other.m_ptr[i];
		}

		return *this;
	}

	~Vector()
	{
		// explicitly call destructors if required.
		if (!m_ptr)
			return;

		for (auto i = 0UL; i < m_numberOfElements; i++)
			m_ptr[i].~T();

		deallocate(m_ptr);
	}

	void reserve(size_t NewCapacity)
	{
		if (NewCapacity <= m_capacity || NewCapacity == 0)
			return;

		auto origptr = m_ptr;
		m_ptr = allocate(NewCapacity);

		for (auto i = 0UL; i < m_numberOfElements; i++)
			new (m_ptr + i) T(origptr[i]);

		m_capacity = NewCapacity;

		if (origptr)
		{
			for (auto i = 0UL; i < m_numberOfElements; i++)
				origptr[i].~T();

			deallocate(origptr);
		}
	}

	void push_back(const T& val)
	{
		auto NewCapacity = m_capacity;
		if (m_numberOfElements + 1 > m_capacity)
		{
			// re allocate.
			if (m_capacity == 0)
				NewCapacity = 1;

			NewCapacity *= 2;

			reserve(NewCapacity);
		}

		new (m_ptr + m_numberOfElements) T(val);
		m_numberOfElements++;
	}

	void push_back(T&& val)
	{
		auto NewCapacity = m_capacity;
		if (m_numberOfElements + 1 > m_capacity)
		{
			// re allocate.
			if (m_capacity == 0)
				NewCapacity = 1;

			NewCapacity *= 2;

			reserve(NewCapacity);
		}

		new (m_ptr + m_numberOfElements) T(std::move(val));
		m_numberOfElements++;
	}

	template<class ...Args>
	T& emplace_back(Args&&... args)
	{
		auto NewCapacity = m_capacity;
		if (m_numberOfElements + 1 > m_capacity)
		{
			// re allocate.
			if (m_capacity == 0)
				NewCapacity = 1;

			NewCapacity *= 2;

			reserve(NewCapacity);
		}

		new (m_ptr + m_numberOfElements) T(forward<Args>(args)...);

		return m_ptr[m_numberOfElements++];
	}

	size_t size()
	{
		return this->m_numberOfElements;
	}

	T& operator[](size_t index)
	{
		return m_ptr[index];
	}

	void Clear()
	{
		destroy(m_ptr, m_numberOfElements);
		m_numberOfElements = 0;
	}

private:

	T* m_ptr;
	size_t m_capacity;
	size_t m_numberOfElements;

	T* allocate(size_t NewCapacity)
	{
		return static_cast<T*>(ExAllocatePoolWithTag(poolType, sizeof(T) * NewCapacity, poolTag));
	}

	void destroy(T* mem, size_t NumElems)
	{
		if (!mem)
			return;

		for (auto i = 0UL; i < NumElems; i++)
			mem[i].~T();
	}

	void deallocate(T* mem)
	{
		if (mem)
			ExFreePool(mem);
	}
};