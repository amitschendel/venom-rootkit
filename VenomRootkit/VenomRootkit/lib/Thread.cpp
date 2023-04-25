#include "Thread.h"

Thread::Thread(ULONG tid) noexcept : m_thread(nullptr) {
    if (PsLookupThreadByThreadId(UlongToHandle(tid), &m_thread) != STATUS_SUCCESS) {
        m_thread = nullptr;
    }
}

Thread::~Thread() {
    if (m_thread) {
        ObDereferenceObject(m_thread);
    }
}

Thread::Thread(Thread&& other) noexcept : m_thread(std::exchange(other.m_thread, nullptr))
{}

Thread& Thread::operator=(Thread&& other) noexcept {
    if (this != &other) {
        if (m_thread) {
            ObDereferenceObject(reinterpret_cast<PVOID>(m_thread));
        }
        m_thread = std::exchange(other.m_thread, nullptr);
    }
    return *this;
}

PETHREAD Thread::get() const noexcept {
    return m_thread;
}