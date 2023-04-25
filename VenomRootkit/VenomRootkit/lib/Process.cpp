#include "Symbols.h"
#include "Memory.h"
#include "../Config.h"
#include "Process.h"


Process::Process(ULONG pid) noexcept : m_process(nullptr) {
    if (PsLookupProcessByProcessId(UlongToHandle(pid), &m_process) != STATUS_SUCCESS) {
        m_process = nullptr;
    }
}

Process::Process(UNICODE_STRING processName) noexcept : m_process(nullptr) {
    auto status = STATUS_SUCCESS;
    const auto processInformation = std::make_unique<SYSTEM_PROCESS_INFO, PagedPool, POOL_TAG>();

    ULONG bytesToAllocate;
    status = ZwQuerySystemInformation(SystemProcessInformation, nullptr, NULL, &bytesToAllocate);
    if (!NT_SUCCESS(status)){
        m_process = nullptr;
    }

    status = ZwQuerySystemInformation(SystemProcessInformation, processInformation.get(), bytesToAllocate, NULL);
    if (!NT_SUCCESS(status)) {
        m_process = nullptr;
    }

    for (auto entry = processInformation.get(); entry->NextEntryOffset != NULL; entry = reinterpret_cast<PSYSTEM_PROCESS_INFO>((reinterpret_cast<PUCHAR>(entry) + entry->NextEntryOffset))) {
        if (RtlEqualUnicodeString(&entry->ImageName, &processName, TRUE)){
            PsLookupProcessByProcessId(entry->UniqueProcessId, &m_process);
            break;
        }
    }
}

Process::~Process() {
    if (m_process) {
        ObDereferenceObject(m_process);
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

HANDLE Process::getPid() const noexcept {
    return PsGetProcessId(m_process);
}
