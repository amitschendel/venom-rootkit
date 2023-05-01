#pragma once

#include <ntifs.h>

enum class Offsets : ULONG
{
	activeProcessLinks = 0x448,
	processLock = 0x438
};
