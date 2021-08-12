#include "IoctlHandlers.h"
#include "TokenHandler.h"

NTSTATUS IoctlHandlers::ElevateToken(PIRP Irp) {
	PEPROCESS Process;
	PACCESS_TOKEN Token;
	auto status = STATUS_SUCCESS;
	auto stack = IoGetCurrentIrpStackLocation(Irp);

	auto pid = (ULONG*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
	if (pid == nullptr || pid < 0) {
		Irp->IoStatus.Information = 0;
		return STATUS_INVALID_PARAMETER;
	}

	status = PsLookupProcessByProcessId(UlongToHandle(*pid), &Process);
	if (!NT_SUCCESS(status)) {
		Irp->IoStatus.Information = 0;
		return status;
	}
	Token = PsReferencePrimaryToken(Process); // Get the process primary token.
	ReplaceToken(Process, Token); // Replace the process token with system token.

	ObDereferenceObject(Token);
	ObDereferenceObject(Process);

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	return status;
}