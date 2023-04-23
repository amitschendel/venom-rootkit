#pragma once

#include "../../Process.h"
#include "../../Token.h"

constexpr ULONG SYSTEM_PID = 4;

class TokenElevator
{
public:
	TokenElevator(Token& token);

	NTSTATUS elevate();

private:
	Token& m_token;
};

