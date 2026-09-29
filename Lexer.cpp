#include "Lexer.h"
#include <cctype>
#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"int", TokenType::KW_INT},
    {"bool", TokenType::KW_BOOL},
    {"string", TokenType::KW_STRING},
    {"true", TokenType::KW_TRUE},
    {"false", TokenType::KW_FALSE},
    {"if", TokenType::KW_IF},
    {"then", TokenType::KW_THEN},
    {"else", TokenType::KW_ELSE},
    {"while", TokenType::KW_WHILE},
    {"read", TokenType::KW_READ},
    {"print", TokenType::KW_PRINT},
    {"let", TokenType::KW_LET},
    {"in", TokenType::KW_IN},
    {"end", TokenType::KW_END},
};

Lexer::Lexer(const std::string& source) : src(source), pos(0), line(1) {}

char Lexer::peekChar() const {
    if (pos >= src.length()) return '\0';
    return src[pos];
}

char Lexer::advanceChar() {
    char c = src[pos++];
    if (c == '\n') line++;
    return c;
}

void Lexer::skipWhitespaceAndComments() {
    while (pos < src.length()) {
        char c = peekChar();
        if (std::isspace(static_cast<unsigned char>(c))) {
            advanceChar();
        } else if (c == '/' && pos + 1 < src.length() && src[pos + 1] == '/') {
            // // 한 줄 주석 지원 (편의상 추가)
            while (pos < src.length() && peekChar() != '\n') advanceChar();
        } else {
            break;
        }
    }
}

Token Lexer::readIdentifierOrKeyword() {
    int startLine = line;
    std::string text;
    while (pos < src.length() &&
           (std::isalnum(static_cast<unsigned char>(peekChar())) || peekChar() == '_')) {
        text += advanceChar();
    }
    auto it = KEYWORDS.find(text);
    if (it != KEYWORDS.end()) {
        return Token(it->second, text, startLine);
    }
    return Token(TokenType::ID, text, startLine);
}

Token Lexer::readNumber() {
    int startLine = line;
    std::string text;
    while (pos < src.length() && std::isdigit(static_cast<unsigned char>(peekChar()))) {
        text += advanceChar();
    }
    return Token(TokenType::INT_LIT, text, startLine);
}

Token Lexer::readString() {
    int startLine = line;
    advanceChar(); // 여는 큰따옴표 소비
    std::string text;
    while (pos < src.length() && peekChar() != '"') {
        text += advanceChar();
    }
    if (pos >= src.length()) {
        throw std::runtime_error("Lexer error (line " + std::to_string(startLine) +
                                  "): 문자열 리터럴이 닫히지 않았습니다.");
    }
    advanceChar(); // 닫는 큰따옴표 소비
    return Token(TokenType::STR_LIT, text, startLine);
}

Token Lexer::readOperator() {
    int startLine = line;
    char c = advanceChar();

    switch (c) {
        case '=':
            if (peekChar() == '=') { advanceChar(); return Token(TokenType::EQ, "==", startLine); }
            return Token(TokenType::ASSIGN, "=", startLine);
        case '!':
            if (peekChar() == '=') { advanceChar(); return Token(TokenType::NEQ, "!=", startLine); }
            return Token(TokenType::NOT, "!", startLine);
        case '<':
            if (peekChar() == '=') { advanceChar(); return Token(TokenType::LE, "<=", startLine); }
            return Token(TokenType::LT, "<", startLine);
        case '>':
            if (peekChar() == '=') { advanceChar(); return Token(TokenType::GE, ">=", startLine); }
            return Token(TokenType::GT, ">", startLine);
        case '+': return Token(TokenType::PLUS, "+", startLine);
        case '-': return Token(TokenType::MINUS, "-", startLine);
        case '*': return Token(TokenType::STAR, "*", startLine);
        case '/': return Token(TokenType::SLASH, "/", startLine);
        case '&': return Token(TokenType::AND, "&", startLine);
        case '|': return Token(TokenType::OR, "|", startLine);
        case '(': return Token(TokenType::LPAREN, "(", startLine);
        case ')': return Token(TokenType::RPAREN, ")", startLine);
        case '{': return Token(TokenType::LBRACE, "{", startLine);
        case '}': return Token(TokenType::RBRACE, "}", startLine);
        case ';': return Token(TokenType::SEMICOLON, ";", startLine);
        default:
            return Token(TokenType::UNKNOWN, std::string(1, c), startLine);
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();

        if (pos >= src.length()) {
            tokens.emplace_back(TokenType::END_OF_INPUT, "", line);
            break;
        }

        char c = peekChar();

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(readIdentifierOrKeyword());
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(readNumber());
        } else if (c == '"') {
            tokens.push_back(readString());
        } else {
            Token t = readOperator();
            if (t.type == TokenType::UNKNOWN) {
                throw std::runtime_error("Lexer error (line " + std::to_string(t.line) +
                                          "): 인식할 수 없는 문자 '" + t.text + "'");
            }
            tokens.push_back(t);
        }
    }

    return tokens;
}
