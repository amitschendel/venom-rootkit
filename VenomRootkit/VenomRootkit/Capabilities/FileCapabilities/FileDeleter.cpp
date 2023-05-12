#include "FileDeleter.h"


FileDeleter::FileDeleter(PFILE_OBJECT const fileObject)
	: m_fileObject(fileObject)
{}

NTSTATUS FileDeleter::deletionCompleteRoutine(PDEVICE_OBJECT deviceObject, PIRP irp, PVOID context) {
	UNREFERENCED_PARAMETER(deviceObject);
	UNREFERENCED_PARAMETER(context);
	irp->UserIosb->Status = irp->IoStatus.Status;
	irp->UserIosb->Information = irp->IoStatus.Information;

	KeSetEvent(irp->UserEvent, IO_NO_INCREMENT, false);
	IoFreeIrp(irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS FileDeleter::deleteFileFromDisk() const{
	KEVENT event;
	const auto deviceObject = IoGetBaseFileSystemDeviceObject(m_fileObject);
	const auto irp = IoAllocateIrp(deviceObject->StackSize, false);

	auto status = closeOpenHandles();
	if (!NT_SUCCESS(status)){
		return status;
	}

	status = flushFileImageSection();
	if (!NT_SUCCESS(status)) {
		return status;
	}
	

	// Set the complete routine that will free the IRP and signal the event.
	KeInitializeEvent(&event, SynchronizationEvent, false);
	IoSetCompletionRoutine(
		irp,
		deletionCompleteRoutine,
		&event,
		true,
		true,
		true);

	FILE_DISPOSITION_INFORMATION fileDisposition;
	fileDisposition.DeleteFile = true;

	IO_STATUS_BLOCK ioStatusBlock;

	irp->AssociatedIrp.SystemBuffer = &fileDisposition;
	irp->UserEvent = &event;
	irp->UserIosb = &ioStatusBlock;
	irp->Tail.Overlay.OriginalFileObject = m_fileObject;
	irp->Tail.Overlay.Thread = KeGetCurrentThread(); // Maybe needs to be in the context of the file.
	irp->RequestorMode = KernelMode;

	const auto stackLocation = IoGetNextIrpStackLocation(irp);
	stackLocation->MajorFunction = IRP_MJ_SET_INFORMATION;
	stackLocation->DeviceObject = deviceObject;
	stackLocation->FileObject = m_fileObject;
	stackLocation->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);
	stackLocation->Parameters.SetFile.FileInformationClass = FileDispositionInformation;
	stackLocation->Parameters.SetFile.FileObject = m_fileObject;

	IoCallDriver(deviceObject, irp);
	KeWaitForSingleObject(&event, Executive, KernelMode, true, nullptr);

	return STATUS_SUCCESS;
}

NTSTATUS FileDeleter::flushFileImageSection() const{
	/*const auto ldrDataEntry = static_cast<PKLDR_DATA_TABLE_ENTRY>(m_driverObject->DriverSection);
	const auto sectionPointer = static_cast<PSECTION>(ldrDataEntry->SectionPointer);
	const auto controlArea = reinterpret_cast<PCONTROL_AREA>(sectionPointer->u1.ControlArea);
	const auto fileObject = reinterpret_cast<PFILE_OBJECT>(static_cast<_EX_FAST_REF*>(controlArea->FilePointer.Object)->Value);*/

	m_fileObject->DeleteAccess = true;

	if(MmFlushImageSection(m_fileObject->SectionObjectPointer, MmFlushForDelete)) {
		return STATUS_SUCCESS;
	}

	return STATUS_UNABLE_TO_DELETE_SECTION;
}

NTSTATUS FileDeleter::closeOpenHandles() const{
    auto status = STATUS_SUCCESS;
    ULONG bytesToAllocate = 0;

    status = ZwQuerySystemInformation(SystemHandleInformation, nullptr, NULL, &bytesToAllocate);
    if (status != STATUS_INFO_LENGTH_MISMATCH) {
        return status;
    }

    const auto systemHandleInformation = UniquePtr<SYSTEM_HANDLE_INFORMATION, PagedPool, POOL_TAG>(
        static_cast<PSYSTEM_HANDLE_INFORMATION>(ExAllocatePoolWithTag(PagedPool, bytesToAllocate, POOL_TAG)));

    status = ZwQuerySystemInformation(SystemHandleInformation, systemHandleInformation.get(), bytesToAllocate, nullptr);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = STATUS_NOT_FOUND;
    for (ULONG i = 0; i < systemHandleInformation->NumberOfHandles; i++) {
	    if (*IoFileObjectType == ObGetObjectType(systemHandleInformation->Handles[i].Object)) {
	        if (m_fileObject == static_cast<PFILE_OBJECT>(systemHandleInformation->Handles[i].Object)) {
                const auto process = Process(systemHandleInformation->Handles[i].UniqueProcessId);
                auto apcState = std::make_unique<KAPC_STATE, NonPagedPool, POOL_TAG>();
                KeStackAttachProcess(process.get(), apcState.get());
                status = NtClose(reinterpret_cast<HANDLE>(systemHandleInformation->Handles[i].HandleValue));
                KeUnstackDetachProcess(apcState.get());
	        }
        }
    }
    return status;
}

