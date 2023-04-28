#include "Apc.h"

Apc::Apc(PKNORMAL_ROUTINE normalRoutine, const Thread& thread, KAPC_ENVIRONMENT enviroment, KPROCESSOR_MODE apcMode)
	:m_apc(std::make_unique<KAPC, NonPagedPool, POOL_TAG>())
	,m_normalRoutineAdress(normalRoutine)
{
	KeInitializeApc(
		m_apc.get(),
		thread.get(),
		enviroment,
		&kernelFreeApc,
		&rundownFreeApc,
		normalRoutine,
		apcMode,
		nullptr
	);
}

Apc::~Apc() {}

Apc::Apc(Apc&& other) noexcept
	:m_apc(std::move(other.m_apc))
	,m_normalRoutineAdress(other.m_normalRoutineAdress)
{}

Apc& Apc::operator=(Apc&& other) noexcept {
	if (this != &other) {
		m_apc = std::move(other.m_apc);
		m_normalRoutineAdress = other.m_normalRoutineAdress;
	}

	return *this;
}

bool Apc::queue() {
	return KeInsertQueueApc(m_apc.get(), nullptr, nullptr, NULL);
}

void Apc::rundownFreeApc(PKAPC) {}

void Apc::kernelFreeApc(PKAPC, PKNORMAL_ROUTINE*, PVOID*, PVOID*, PVOID*) {}
