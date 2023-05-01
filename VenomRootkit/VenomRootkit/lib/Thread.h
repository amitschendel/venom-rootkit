#pragma once

#include <ntifs.h>

#include "Utils.h"

class Thread
{
public:
    explicit Thread(ULONG tid) noexcept;
    ~Thread();
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;
    Thread(Thread&& other) noexcept;
    Thread& operator=(Thread&& other) noexcept;

    PETHREAD get() const noexcept;

private:
    PETHREAD m_thread;
};
