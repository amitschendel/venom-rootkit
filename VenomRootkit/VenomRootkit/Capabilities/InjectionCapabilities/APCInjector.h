#pragma once

#include <ntifs.h>

#include "../../lib/Process.h"
#include "../../lib/Thread.h"
#include "../../lib/Buffer.h"
#include "../../ApcHandler.h"

class APCInjector
{
public:
	APCInjector(Process& process, NonPagedBuffer& shellcode);

	NTSTATUS inject();

private:
	NTSTATUS findInjectableThread(PULONG tid);

	Process& m_process;
	NonPagedBuffer& m_shellcode;
};
