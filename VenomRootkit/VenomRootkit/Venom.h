#pragma once
#include <ntifs.h> 

void VenomUnload(PDRIVER_OBJECT driverObject);
NTSTATUS VenomCreateClose(PDEVICE_OBJECT deviceObject, PIRP irp);
NTSTATUS VenomDeviceControl(PDEVICE_OBJECT deviceObject, PIRP irp);
