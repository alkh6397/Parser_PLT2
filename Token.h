#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// ==================================================
// 언어 S에서 사용하는 토큰 종류
// ==================================================
enum class TokenType {
    // 리터럴 / 식별자
    ID, INT_LIT, STR_LIT,

    // 키워드
    KW_INT, KW_BOOL, KW_STRING,
    KW_TRUE, KW_FALSE,
    KW_IF, KW_THEN, KW_ELSE,
    KW_WHILE, KW_READ, KW_PRINT,
    KW_LET, KW_IN, KW_END,

    // 연산자
    ASSIGN,        // =
    EQ, NEQ,       // == !=
    LT, GT, LE, GE,// < > <= >=
    PLUS, MINUS,   // + -
    STAR, SLASH,   // * /
    AND, OR, NOT,  // & | !

    // 구두점
    LPAREN, RPAREN, // ( )
    LBRACE, RBRACE, // { }
    SEMICOLON,      // ;

    END_OF_INPUT,
    UNKNOWN
};

struct Token {
    TokenType type;
    std::string text;   // 원본 텍스트 (식별자명, 리터럴 텍스트 등)
    int line;

    Token() : type(TokenType::UNKNOWN), text(""), line(0) {}
    Token(TokenType t, const std::string& s, int l)
        : type(t), text(s), line(l) {}
};

// 토큰 타입을 사람이 읽을 수 있는 문자열로 (에러 메시지용)
inline std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::ID: return "identifier";
        case TokenType::INT_LIT: return "int literal";
        case TokenType::STR_LIT: return "string literal";
        case TokenType::KW_INT: return "'int'";
        case TokenType::KW_BOOL: return "'bool'";
        case TokenType::KW_STRING: return "'string'";
        case TokenType::KW_TRUE: return "'true'";
        case TokenType::KW_FALSE: return "'false'";
        case TokenType::KW_IF: return "'if'";
        case TokenType::KW_THEN: return "'then'";
        case TokenType::KW_ELSE: return "'else'";
        case TokenType::KW_WHILE: return "'while'";
        case TokenType::KW_READ: return "'read'";
        case TokenType::KW_PRINT: return "'print'";
        case TokenType::KW_LET: return "'let'";
        case TokenType::KW_IN: return "'in'";
        case TokenType::KW_END: return "'end'";
        case TokenType::ASSIGN: return "'='";
        case TokenType::EQ: return "'=='";
        case TokenType::NEQ: return "'!='";
        case TokenType::LT: return "'<'";
        case TokenType::GT: return "'>'";
        case TokenType::LE: return "'<='";
        case TokenType::GE: return "'>='";
        case TokenType::PLUS: return "'+'";
        case TokenType::MINUS: return "'-'";
        case TokenType::STAR: return "'*'";
        case TokenType::SLASH: return "'/'";
        case TokenType::AND: return "'&'";
        case TokenType::OR: return "'|'";
        case TokenType::NOT: return "'!'";
        case TokenType::LPAREN: return "'('";
        case TokenType::RPAREN: return "')'";
        case TokenType::LBRACE: return "'{'";
        case TokenType::RBRACE: return "'}'";
        case TokenType::SEMICOLON: return "';'";
        case TokenType::END_OF_INPUT: return "end of input";
        default: return "unknown token";
    }
}

#endif
