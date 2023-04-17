#include "Process.h"


Process::Process(ULONG pid) noexcept : m_handle(nullptr) {
    PEPROCESS eprocess;
    if (PsLookupProcessByProcessId(UlongToHandle(pid), &eprocess) == STATUS_SUCCESS) {
        m_handle = reinterpret_cast<HANDLE>(eprocess);
    }
}

Process::~Process() {
    if (m_handle) {
        ObDereferenceObject(reinterpret_cast<PVOID>(m_handle));
    }
}

Process::Process(Process&& other) noexcept : m_handle(std::exchange(other.m_handle, nullptr))
{}

Process& Process::operator=(Process&& other) noexcept {
    if (this != &other) {
        if (m_handle) {
            ObDereferenceObject(reinterpret_cast<PVOID>(m_handle));
        }
        m_handle = std::exchange(other.m_handle, nullptr);
    }
    return *this;
}

HANDLE Process::get() const noexcept {
    return m_handle;
}
