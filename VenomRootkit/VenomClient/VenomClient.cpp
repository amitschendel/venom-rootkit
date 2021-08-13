#include <Windows.h>
#include <iostream>
#include <string>

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

	//BOOL success = DeviceIoControl(hFile, IOCTL_PROCESS_GUARD_FORBIDDEN_PROCESS, &data, sizeof(data), nullptr, 0, &returned, nullptr);
}