#pragma once

#include <ntifs.h>

#include "../../Process.h"
#include "lib/Utils.h"

class Token {
public:
    explicit Token(Process& process) noexcept;
    ~Token();
    Token(const Token&) = delete;
    Token& operator=(const Token&) = delete;
    Token(Token&& other) noexcept;
    Token& operator=(Token&& other) noexcept;

    PACCESS_TOKEN get() const noexcept;

private:
    PACCESS_TOKEN m_accessToken;
};
