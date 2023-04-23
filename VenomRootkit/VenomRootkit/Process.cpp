#include "Process.h"


Process::Process(ULONG pid) noexcept : m_process(nullptr) {
    if (PsLookupProcessByProcessId(UlongToHandle(pid), &m_process) != STATUS_SUCCESS) {
        m_process = nullptr;
    }
}

Process::~Process() {
    if (m_process) {
        ObDereferenceObject(reinterpret_cast<PVOID>(m_process));
    }
}

Process::Process(Process&& other) noexcept : m_process(std::exchange(other.m_process, nullptr))
{}

Process& Process::operator=(Process&& other) noexcept {
    if (this != &other) {
        if (m_process) {
            ObDereferenceObject(reinterpret_cast<PVOID>(m_process));
        }
        m_process = std::exchange(other.m_process, nullptr);
    }
    return *this;
}

PEPROCESS Process::get() const noexcept {
    return m_process;
}
