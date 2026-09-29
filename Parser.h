#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include "Token.h"
#include "AST.h"

// ==================================================
// Parser: 토큰 스트림을 받아 재귀 하향 방식으로
// 언어 S의 AST를 구성한다. (계산은 하지 않고
// 트리 구조만 만든다 — 실습 #2의 목표)
// ==================================================
class Parser {
private:
    std::vector<Token> tokens;
    size_t pos;

    const Token& current() const;
    bool check(TokenType t) const;
    Token match(TokenType expected); // 현재 토큰이 expected면 소비 후 리턴, 아니면 에러
    void error(const std::string& msg) const;

    bool isTypeStart() const;   // int/bool/string
    bool isStmtStart() const;   // id, {, if, while, read, print, let

    // ---- 수식 문법 ----
    std::unique_ptr<Expr> expr();
    std::unique_ptr<Expr> bexp();
    std::unique_ptr<Expr> aexp();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    bool isRelop() const;
    std::string relopText();

    // ---- 문장/선언 문법 ----
    Type typeSpec();
    std::unique_ptr<Decl> decl();
    std::unique_ptr<Stmt> stmt();
    std::vector<std::unique_ptr<Stmt>> stmtsList();
    std::vector<std::unique_ptr<Decl>> declsList();
    std::unique_ptr<Command> command();

public:
    explicit Parser(std::vector<Token> toks);
    std::unique_ptr<Program> parseProgram();
};

#endif
