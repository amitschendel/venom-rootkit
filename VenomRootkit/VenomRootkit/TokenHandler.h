#pragma once
#include <Ntifs.h>
namespace TokenHandler {
	void ReplaceToken(PEPROCESS Process, PACCESS_TOKEN Token);
}
