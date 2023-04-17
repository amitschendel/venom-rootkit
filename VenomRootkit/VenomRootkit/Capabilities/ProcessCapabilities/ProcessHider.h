#pragma once

#include <ntifs.h>

#include "../../Process.h"

class ProcessHider
{
public:
	ProcessHider(Process& process);

	NTSTATUS hide();

private:
	Process& m_process;
};
