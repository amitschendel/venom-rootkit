#pragma once

#include <ntifs.h>
#include "Venom.h"
#include "ShellcodeHandler.h"

namespace ApcHandler {

	extern EX_RUNDOWN_REF g_rundown_protection;

	typedef enum _KAPC_ENVIRONMENT {
		OriginalApcEnvironment,
		AttachedApcEnvironment,
		CurrentApcEnvironment,
		InsertApcEnvironment
	} KAPC_ENVIRONMENT;

	typedef
		VOID
		(*PKNORMAL_ROUTINE) (
			IN PVOID NormalContext,
			IN PVOID SystemArgument1,
			IN PVOID SystemArgument2
			);

	typedef
		VOID
		(*PKKERNEL_ROUTINE) (
			IN PKAPC Apc,
			IN OUT PKNORMAL_ROUTINE* NormalRoutine,
			IN OUT PVOID* NormalContext,
			IN OUT PVOID* SystemArgument1,
			IN OUT PVOID* SystemArgument2
			);

	typedef
		VOID
		(*PKRUNDOWN_ROUTINE) (
			IN  PKAPC Apc
			);

	extern "C"
		VOID
		KeInitializeApc(
			IN  PKAPC Apc,
			IN  PKTHREAD Thread,
			IN  KAPC_ENVIRONMENT Environment,
			IN  PKKERNEL_ROUTINE KernelRoutine,
			IN  PKRUNDOWN_ROUTINE RundownRoutine OPTIONAL,
			IN  PKNORMAL_ROUTINE NormalRoutine OPTIONAL,
			IN  KPROCESSOR_MODE ApcMode OPTIONAL,
			IN  PVOID NormalContext OPTIONAL
		);

	extern "C"
		BOOLEAN
		KeInsertQueueApc(
			IN  PKAPC Apc,
			IN  PVOID SystemArgument1,
			IN  PVOID SystemArgument2,
			IN  KPRIORITY Increment
		);

	extern "C"
		NTSYSAPI
		NTSTATUS
		NTAPI
		ZwProtectVirtualMemory(
			IN HANDLE ProcessHandle,
			IN OUT PVOID * BaseAddress,
			IN OUT PULONG NumberOfBytesToProtect,
			IN ULONG NewAccessProtection,
			OUT PULONG OldAccessProtection
		);

	extern "C"
		NTKERNELAPI
		PVOID
		NTAPI
		PsGetProcessWow64Process(
			_In_ PEPROCESS Process
		);

	extern "C"
		NTKERNELAPI
		BOOLEAN
		NTAPI
		PsIsProtectedProcess(
			_In_ PEPROCESS Process
		);

	extern "C"
	BOOLEAN
		KeTestAlertThread(
			IN  KPROCESSOR_MODE AlertMode
		);

	void kernelFreeKapc(PKAPC apc, PKNORMAL_ROUTINE*, PVOID*, PVOID*, PVOID*);

	void rundownFreeKapc(PKAPC apc);

	void normalInjectCode(PVOID, PVOID, PVOID);
}