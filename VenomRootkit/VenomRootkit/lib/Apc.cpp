#include "Apc.h"

Apc::Apc(PKNORMAL_ROUTINE normalRoutine, const Thread& thread, KAPC_ENVIRONMENT enviroment, KPROCESSOR_MODE apcMode)
	:m_apc(std::make_unique<KAPC, NonPagedPool, POOL_TAG>())
	,m_normalRoutineAdress(normalRoutine)
{
	ExInitializeRundownProtection(&m_rundownProtection);
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

Apc::~Apc() {
	ExWaitForRundownProtectionRelease(&m_rundownProtection);
}

Apc::Apc(Apc&& other) noexcept
	:m_apc(std::move(other.m_apc))
	,m_normalRoutineAdress(other.m_normalRoutineAdress)
{}

Apc& Apc::operator=(Apc&& other) noexcept {
	if (this != &other) {
		ExReleaseRundownProtection(&m_rundownProtection);

		m_rundownProtection = other.m_rundownProtection;
		m_apc = std::move(other.m_apc);
		m_normalRoutineAdress = other.m_normalRoutineAdress;
	}

	return *this;
}

bool Apc::queue() {
	if (ExAcquireRundownProtection(&m_rundownProtection)) {
		if (!KeInsertQueueApc(m_apc.get(), nullptr, nullptr, NULL)) {
			__debugbreak();
			ExReleaseRundownProtection(&m_rundownProtection);
			return 0;
		}
	}

	return 1;
}

void Apc::rundownFreeApc(PKAPC) {
	ExReleaseRundownProtection(&m_rundownProtection);
}

void Apc::kernelFreeApc(PKAPC, PKNORMAL_ROUTINE*, PVOID*, PVOID*, PVOID*) {
	ExReleaseRundownProtection(&m_rundownProtection);
}
