#pragma once

#include <ntifs.h>

#include "../Config.h"
#include "Utils.h"

template <typename T, POOL_TYPE poolType, ULONG poolTag = POOL_TAG>
class UniquePtr {
public:
    UniquePtr() : m_ptr(nullptr) {}

    explicit UniquePtr(T* ptr) : m_ptr(ptr) {}

    ~UniquePtr() {
        if (m_ptr) {
            ExFreePoolWithTag(m_ptr, poolTag);
        }
    }

    UniquePtr(const UniquePtr&) = delete;

    UniquePtr& operator=(const UniquePtr&) = delete;

    UniquePtr(UniquePtr&& other) noexcept
        : m_ptr(std::exchange(other.m_ptr, nullptr)) {}

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            if (m_ptr) {
                ExFreePoolWithTag(m_ptr, poolTag);
            }

            m_ptr = std::exchange(other.m_ptr, nullptr);
        }
        return *this;
    }

    T& operator*() const {
        return *m_ptr;
    }

    T* operator->() const {
        return m_ptr;
    }

    T* get() const {
        return m_ptr;
    }

    T* release() {
        return std::exchange(m_ptr, nullptr);
    }

    void reset(T* ptr = nullptr) {
        if (m_ptr) {
            ExFreePoolWithTag(m_ptr, poolTag);
        }

        m_ptr = p;
    }

private:
    T* m_ptr;
};
