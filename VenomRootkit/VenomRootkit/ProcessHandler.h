#pragma once
#include <Ntifs.h>

namespace ProcessHandler {

	NTSTATUS UnlinkActiveProcessLinks(ULONG pid);
}