#pragma once
#include <Ntifs.h>

namespace ProcHandler {

	NTSTATUS UnlinkActiveProcessLinks(ULONG pid);
}