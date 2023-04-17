#pragma once

#include <ntifs.h>

#include "lib/Utils.h"

class Process {
public:
    explicit Process(ULONG pid) noexcept;
    ~Process();
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    Process(Process&& other) noexcept;
    Process& operator=(Process&& other) noexcept;

    HANDLE get() const noexcept;

private:
    HANDLE m_handle;
};