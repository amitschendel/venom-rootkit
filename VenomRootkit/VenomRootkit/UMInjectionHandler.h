#pragma once
#include <ntifs.h>
#include "ApcHandler.h"
#include "NativeStructs.h"


namespace UMInjectionHandler {

	EXTERN_C NTKERNELAPI PVOID NTAPI PsGetThreadTeb(PETHREAD Thread);

	EXTERN_C NTKERNELAPI PVOID NTAPI PsGetProcessWow64Process(PEPROCESS Process);

	EXTERN_C NTKERNELAPI BOOLEAN NTAPI PsIsProtectedProcess(PEPROCESS Process);

	EXTERN_C
	NTSYSAPI
		NTSTATUS
		NTAPI
		ZwQuerySystemInformation(
			IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
			OUT PVOID SystemInformation,
			IN ULONG SystemInformationLength,
			OUT PULONG ReturnLength OPTIONAL
		);

	// Checks if the thread is valid for APC injection (Taken from BB Rootkit).
	bool shouldSkipThread(PETHREAD pThread, BOOLEAN isWow64);

	// Gets the first valid thread for apc injection from the given process (Taken from BB Rootkit).
	NTSTATUS getProcessThread(PEPROCESS pProcess, PETHREAD* ppThread);

	// Find process id by name
	HANDLE getProcessId(UNICODE_STRING processName);

	NTSTATUS injectDll(ULONG pid);

}