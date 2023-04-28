#pragma once
#include <ntifs.h> 

void VenomUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS VenomCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS VenomDeviceControl(PDEVICE_OBJECT, PIRP Irp);
