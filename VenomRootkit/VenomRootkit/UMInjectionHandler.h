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

	// Gets the first valid thread for apc injection from the given process.
	NTSTATUS getProcessThread(PEPROCESS pProcess, PETHREAD* ppThread);

	// Find process id by name.
	HANDLE getProcessId(UNICODE_STRING processName);

	// Inject dll to the given process by using APC injection.
	NTSTATUS injectDll(ULONG pid);

}