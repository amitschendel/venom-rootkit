#include "UMInjectionHandler.h"
#include "Venom.h"

bool UMInjectionHandler::shouldSkipThread(PETHREAD pThread, BOOLEAN isWow64)
{
    PUCHAR pTeb64 = reinterpret_cast<PUCHAR>(PsGetThreadTeb(pThread));
    if (!pTeb64)
        return TRUE;

    // Skip GUI treads. APC to GUI thread causes ZwUserGetMessage to fail
    // TEB64 + 0x78  = Win32ThreadInfo
    if (*(PULONG64)(pTeb64 + 0x78) != 0)
        return TRUE;

    // Skip threads with no ActivationContext
    // Skip threads with no TLS pointer
    if (isWow64)
    {
        PUCHAR pTeb32 = pTeb64 + 0x2000;

        // TEB32 + 0x1A8 = ActivationContextStackPointer
        if (*(PULONG32)(pTeb32 + 0x1A8) == 0)
            return TRUE;

        // TEB64 + 0x2C = ThreadLocalStoragePointer
        if (*(PULONG32)(pTeb32 + 0x2C) == 0)
            return TRUE;
    }
    else
    {
        // TEB64 + 0x2C8 = ActivationContextStackPointer
        if (*(PULONG64)(pTeb64 + 0x2C8) == 0)
            return TRUE;

        // TEB64 + 0x58 = ThreadLocalStoragePointer
        if (*(PULONG64)(pTeb64 + 0x58) == 0)
            return TRUE;
    }

    return FALSE;
}

NTSTATUS UMInjectionHandler::getProcessThread(PEPROCESS pProcess, PETHREAD* ppThread)
{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE pid = PsGetProcessId(pProcess);
    PVOID pBuf = ExAllocatePoolWithTag(NonPagedPool, 1024 * 1024, DRIVER_TAG);
    PSYSTEM_PROCESS_INFO pInfo = (PSYSTEM_PROCESS_INFO)pBuf;

    ASSERT(ppThread != NULL);
    if (ppThread == NULL)
        return STATUS_INVALID_PARAMETER;

    if (!pInfo)
    {
        DbgPrint("BlackBone: %s: Failed to allocate memory for process list\n", __FUNCTION__);
        return STATUS_NO_MEMORY;
    }

    // Get the process thread list
    status = ZwQuerySystemInformation(SystemProcessInformation, pInfo, 1024 * 1024, NULL);
    if (!NT_SUCCESS(status))
    {
        ExFreePoolWithTag(pBuf, DRIVER_TAG);
        return status;
    }

    // Find target thread
    if (NT_SUCCESS(status))
    {
        status = STATUS_NOT_FOUND;
        for (;;)
        {
            if (pInfo->UniqueProcessId == pid)
            {
                status = STATUS_SUCCESS;
                break;
            }
            else if (pInfo->NextEntryOffset)
                pInfo = (PSYSTEM_PROCESS_INFO)((PUCHAR)pInfo + pInfo->NextEntryOffset);
            else
                break;
        }
    }

    BOOLEAN wow64 = PsGetProcessWow64Process(pProcess) != NULL;

    // Reference target thread
    if (NT_SUCCESS(status))
    {
        status = STATUS_NOT_FOUND;

        // Get first thread
        for (ULONG i = 0; i < pInfo->NumberOfThreads; i++)
        {
            // Skip current thread
            if (/*pInfo->Threads[i].WaitReason == Suspended ||
                 pInfo->Threads[i].ThreadState == 5 ||*/
                pInfo->Threads[i].ClientId.UniqueThread == PsGetCurrentThreadId())
            {
                continue;
            }

            status = PsLookupThreadByThreadId(pInfo->Threads[i].ClientId.UniqueThread, ppThread);

            // Skip specific threads
            //
            //if (*ppThread && shouldSkipThread(*ppThread, wow64))
            if (*ppThread)
            {
                if (wow64 || PsIsProtectedProcess(pProcess))
                {
                    ObDereferenceObject(*ppThread);
                    *ppThread = NULL;
                    continue;
                }
            }

            break;
        }
    }
    else
        DbgPrint("BlackBone: %s: Failed to locate process\n", __FUNCTION__);

    if (pBuf)
        ExFreePoolWithTag(pBuf, DRIVER_TAG);

    // No suitable thread
    if (!*ppThread)
        status = STATUS_NOT_FOUND;

    return status;
}

NTSTATUS UMInjectionHandler::injectDll(ULONG pid)
{
    PEPROCESS pProcess;
    PETHREAD pThread;
    auto status = STATUS_SUCCESS;

    status = PsLookupProcessByProcessId(UlongToHandle(pid), &pProcess);

    if (!NT_SUCCESS(status))
        return status;

    status = getProcessThread(pProcess, &pThread);

    if (!NT_SUCCESS(status))
        return status;

    PKAPC apc = (PKAPC)ExAllocatePoolWithTag(NonPagedPool, sizeof(KAPC), DRIVER_TAG);

    if (!apc) {
        KdPrint(("[-] Could not queue a kernel APC due to insufficient memory.\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ApcHandler::KeInitializeApc(
        apc,
        pThread,
        ApcHandler::OriginalApcEnvironment,
        &ApcHandler::kernelFreeKapc,
        &ApcHandler::rundownFreeKapc,
        &ApcHandler::normalInjectCode,
        KernelMode,
        nullptr
    );

    if (::ExAcquireRundownProtection(&ApcHandler::g_rundown_protection)) {
        auto inserted = ApcHandler::KeInsertQueueApc(
            apc,
            nullptr,
            nullptr,
            0
        );

        if (!inserted) {
            ExFreePoolWithTag(apc, DRIVER_TAG);
            ::ExReleaseRundownProtection(&ApcHandler::g_rundown_protection);

            KdPrint(("[-] Could not insert a kernel APC.\n"));
            return STATUS_INTERNAL_ERROR;
        }
    }
    return STATUS_SUCCESS;
}