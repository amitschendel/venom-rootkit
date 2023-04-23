#include "Token.h"

Token::Token(Process& process) noexcept {
	m_accessToken = PsReferencePrimaryToken(process.get());
}

Token::~Token() {
	if (m_accessToken){
		ObDereferenceObject(m_accessToken);
	}
}

Token::Token(Token&& other) noexcept : m_accessToken(std::exchange(other.m_accessToken, nullptr))
{}

Token& Token::operator=(Token&& other) noexcept {
    if (this != &other) {
        if (m_accessToken) {
            ObDereferenceObject(m_accessToken);
        }
        m_accessToken = std::exchange(other.m_accessToken, nullptr);
    }
    return *this;
}

PACCESS_TOKEN Token::get() const noexcept {
    return m_accessToken;
}
