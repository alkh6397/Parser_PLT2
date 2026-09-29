#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "Token.h"

// ==================================================
// Lexer: 소스코드 문자열을 Token들의 리스트로 변환
// (Parser가 문자 단위가 아닌 토큰 단위로 동작하도록)
// ==================================================
class Lexer {
private:
    std::string src;
    size_t pos;
    int line;

    char peekChar() const;
    char advanceChar();
    void skipWhitespaceAndComments();
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readString();
    Token readOperator();

public:
    explicit Lexer(const std::string& source);

    // 소스 전체를 한 번에 토큰화해서 리턴
    std::vector<Token> tokenize();
};

#endif
