#pragma once

#include "../../lib/Process.h"

constexpr ULONG SYSTEM_PID = 4;

class TokenElevator
{
public:
	TokenElevator(Process& processToElevate);

	NTSTATUS elevate();

private:
	Process& m_processToElevate;
};
