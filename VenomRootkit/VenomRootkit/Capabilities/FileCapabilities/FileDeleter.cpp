#include "FileDeleter.h"
#include "..\..\lib\UniquePtr.h"


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
	auto status = closeOpenHandles();
	if (!NT_SUCCESS(status)) {
		return status;
	}

	m_fileObject->DeleteAccess = true;

	PVOID imageSectionObject = nullptr;
	PVOID dataSectionObject = nullptr;
	PVOID sharedCacheMap = nullptr;

	if (nullptr != m_fileObject->SectionObjectPointer) {
		imageSectionObject = m_fileObject->SectionObjectPointer->ImageSectionObject;
		dataSectionObject = m_fileObject->SectionObjectPointer->DataSectionObject;
		sharedCacheMap = m_fileObject->SectionObjectPointer->SharedCacheMap;

		m_fileObject->SectionObjectPointer->ImageSectionObject = nullptr;
		m_fileObject->SectionObjectPointer->DataSectionObject = nullptr;
		m_fileObject->SectionObjectPointer->SharedCacheMap = nullptr;
	}

	OBJECT_ATTRIBUTES objectAttributes;
	UNICODE_STRING filePath;
	const auto length = (wcslen(FILE_PREFIX) + 1) * sizeof(wchar_t);
	wchar_t fullFilePath[256] = {};
	RtlCopyMemory(fullFilePath, FILE_PREFIX, length);
	RtlCopyMemory(fullFilePath + wcslen(FILE_PREFIX), m_fileObject->FileName.Buffer, wcslen(m_fileObject->FileName.Buffer) * sizeof(wchar_t));
	RtlInitUnicodeString(&filePath, fullFilePath);
	InitializeObjectAttributes(&objectAttributes, &filePath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, nullptr, nullptr);
	HANDLE fileHandle;
	IO_STATUS_BLOCK ioStatusBlock;
	status = ZwCreateFile(&fileHandle, FILE_READ_ATTRIBUTES, &objectAttributes, &ioStatusBlock, nullptr, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, FILE_OPEN, 0, nullptr, 0);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = ObReferenceObjectByHandle(fileHandle, FILE_READ_ATTRIBUTES, *IoFileObjectType, KernelMode, (PVOID*)&m_fileObject, nullptr);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	status = sendDeleteIrp();

	if (nullptr != m_fileObject->SectionObjectPointer) {
		m_fileObject->SectionObjectPointer->ImageSectionObject = imageSectionObject;
		m_fileObject->SectionObjectPointer->DataSectionObject = dataSectionObject;
		m_fileObject->SectionObjectPointer->SharedCacheMap = sharedCacheMap;
	}

	status = ObCloseHandle(fileHandle, KernelMode);
	if (!NT_SUCCESS(status)) {
		return status;
	}

	return STATUS_SUCCESS;
}

NTSTATUS FileDeleter::sendDeleteIrp() const {
	KEVENT event;
	const auto deviceObject = IoGetBaseFileSystemDeviceObject(m_fileObject);
	const auto irp = IoAllocateIrp(deviceObject->StackSize, false);
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
	irp->Tail.Overlay.Thread = static_cast<PETHREAD>(KeGetCurrentThread());
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

NTSTATUS FileDeleter::closeOpenHandles() const{
    auto status = STATUS_SUCCESS;
	const auto bytesToAllocate = 1024 * 1024 * 2; //TODO: Find a better way. For some reason a call with null returns incorrect size.

    const auto systemHandleInformation = UniquePtr<SYSTEM_HANDLE_INFORMATION, PagedPool, POOL_TAG>(
		reinterpret_cast<PSYSTEM_HANDLE_INFORMATION>(ExAllocatePoolWithTag(PagedPool, bytesToAllocate, POOL_TAG)));
    status = ZwQuerySystemInformation(SystemHandleInformation, systemHandleInformation.get(), bytesToAllocate, nullptr);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = STATUS_NOT_FOUND;
    for (ULONG i = 0; i < systemHandleInformation->NumberOfHandles; i++) {
	    if (*IoFileObjectType == ObGetObjectType(systemHandleInformation->Handles[i].Object)) {
			POBJECT_NAME_INFORMATION objectNameInformation;
			status = IoQueryFileDosDeviceName(static_cast<PFILE_OBJECT>(systemHandleInformation->Handles[i].Object), &objectNameInformation);
			if (!NT_SUCCESS(status)) {
				continue;
			}
			if (0 == wcscmp(m_fileObject->FileName.Buffer, objectNameInformation->Name.Buffer)) {
                const auto process = Process(systemHandleInformation->Handles[i].UniqueProcessId);
                auto apcState = std::make_unique<KAPC_STATE, NonPagedPool, POOL_TAG>();
                KeStackAttachProcess(process.get(), apcState.get());
                status = ZwClose(reinterpret_cast<HANDLE>(systemHandleInformation->Handles[i].HandleValue));
                KeUnstackDetachProcess(apcState.get());
	        }
        }
    }

    return status;
}
