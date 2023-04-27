#pragma once

#include <ntifs.h>

#include "Symbols.h"
#include "Utils.h"
#include "Memory.h"
#include "../../ShellcodeHandler.h"
#include "UniquePtr.h"
#include "Thread.h"

class Apc
{
public:
	Apc(PKNORMAL_ROUTINE normalRoutine, const Thread& thread, KAPC_ENVIRONMENT enviroment, KPROCESSOR_MODE apcMode);
	~Apc();
	Apc(const Apc&) = delete;
	Apc& operator=(const Apc&) = delete;
	Apc(Apc&& other) noexcept;
	Apc& operator=(Apc&& other) noexcept;

	bool queue();

private:
	static void rundownFreeApc(PKAPC);
	static void kernelFreeApc(PKAPC, PKNORMAL_ROUTINE*, PVOID*, PVOID*, PVOID*);

	inline static EX_RUNDOWN_REF m_rundownProtection;
	UniquePtr<KAPC, NonPagedPool> m_apc;
	PKNORMAL_ROUTINE m_normalRoutineAdress;
};
