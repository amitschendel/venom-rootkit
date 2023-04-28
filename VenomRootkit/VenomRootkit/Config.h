#pragma once

#include <ntifs.h>

constexpr auto POOL_TAG = 'VMR';

constexpr UNICODE_STRING VENOM_HOST_PROCESS = RTL_CONSTANT_STRING(L"explorer.exe");
