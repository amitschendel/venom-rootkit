#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <iostream>
#include <string>

namespace CommunicationHandler {

	// Need to build the protobuf protocol structs.

	constexpr bool STATUS_SUCCESS = true;
	constexpr bool STATUS_FAILURE = false;

	class Communicator
	{
		private:
			const int  MAXIMUM_CONNECTION_ATTEMPTS = 15;
			SOCKADDR_IN cncInformation = { 0 };
			SOCKET connectionSocket = INVALID_SOCKET;
			WSADATA wsaData = { 0 };

		public:
			Communicator(std::string cncIp, int cncPort);

			bool connectToCnc();
			void SendTelemetry();
			bool isConnectionValid();
			void pullCommand();
			void sendDataToCnc();
			bool DisconnectFromCnc();

			~Communicator();

		};

}