#pragma once
#include <ntifs.h> 

constexpr auto PTR_ALIGN = 0xfffffffffffffff0;

void VenomUnload(PDRIVER_OBJECT driverObject);
NTSTATUS VenomCreateClose(PDEVICE_OBJECT deviceObject, PIRP irp);
NTSTATUS VenomDeviceControl(PDEVICE_OBJECT deviceObject, PIRP irp);
