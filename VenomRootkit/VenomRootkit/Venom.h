#pragma once
#include <Ntifs.h> 

#define DRIVER_PREFIX "VR"
#define DRIVER_TAG 'VMR'

void VenomUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS VenomCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS VenomDeviceControl(PDEVICE_OBJECT, PIRP Irp);
