#pragma once

#include <ntifs.h>

#include "../../lib/Symbols.h"
#include "../../lib/UniquePtr.h"
#include "../../lib/Process.h"
#include "../../lib/Memory.h"

constexpr auto FILE_PREFIX = L"\\??\\C:\\"; //TODO: Support volumes.

class FileDeleter
{
public:
	FileDeleter(PFILE_OBJECT const fileObject);

	[[nodiscard]] NTSTATUS deleteFileFromDisk() const;

private:
	static NTSTATUS deletionCompleteRoutine(PDEVICE_OBJECT deviceObject, PIRP irp, PVOID context);
	[[nodiscard]] NTSTATUS sendDeleteIrp() const;
	[[nodiscard]] NTSTATUS closeOpenHandles() const;

	PFILE_OBJECT m_fileObject;
};
