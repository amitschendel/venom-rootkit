#pragma once

#include <ntifs.h>

#include "Utils.h"

class Process {
public:
    explicit Process(ULONG pid) noexcept;
    explicit Process(UNICODE_STRING processName) noexcept;
    ~Process();
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    Process(Process&& other) noexcept;
    Process& operator=(Process&& other) noexcept;

    PEPROCESS get() const noexcept;
    HANDLE getPid() const noexcept;

private:
    PEPROCESS m_process;
};
