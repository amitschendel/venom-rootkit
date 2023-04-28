#pragma once

#include <ntifs.h>

#include "Shellcode.h"
#include "../../lib/Process.h"
#include "../../lib/Thread.h"
#include "../../lib/Buffer.h"
#include "../../lib/Apc.h"

class APCInjector
{
public:
	APCInjector(Process& process);

	NTSTATUS inject();

private:
	NTSTATUS findInjectableThread(PULONG tid);
	/*
	* Queues a kernel APC that queues the usermode APC in order to avoide an event log.
	* @return NTSTATUS
	*/
	NTSTATUS queueKernelApc();

	/*
	* Injects the shellcode to the process and queues the usermode APC.
	* @return void
	*/
	static void normalRoutine(PVOID, PVOID, PVOID);

	Process& m_process;
};
