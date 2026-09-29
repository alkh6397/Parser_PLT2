#include "Parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> toks) : tokens(std::move(toks)), pos(0) {}

const Token& Parser::current() const {
    return tokens[pos];
}

bool Parser::check(TokenType t) const {
    return current().type == t;
}

Token Parser::match(TokenType expected) {
    if (current().type != expected) {
        error("expected " + tokenTypeName(expected) +
              " but found " + tokenTypeName(current().type) +
              " ('" + current().text + "')");
    }
    Token t = current();
    pos++;
    return t;
}

void Parser::error(const std::string& msg) const {
    throw std::runtime_error("Syntax error (line " + std::to_string(current().line) +
                              "): " + msg);
}

bool Parser::isTypeStart() const {
    TokenType t = current().type;
    return t == TokenType::KW_INT || t == TokenType::KW_BOOL || t == TokenType::KW_STRING;
}

bool Parser::isStmtStart() const {
    switch (current().type) {
        case TokenType::ID:
        case TokenType::LBRACE:
        case TokenType::KW_IF:
        case TokenType::KW_WHILE:
        case TokenType::KW_READ:
        case TokenType::KW_PRINT:
        case TokenType::KW_LET:
            return true;
        default:
            return false;
    }
}

// ==================================================
// <factor> → [ - ] ( number | (<expr>) | id ) | strliteral
//
// * 참고: 강의자료 EBNF는 괄호 안을 <aexp>로만 명시하지만,
//   실습 입력 예제에 "flag = (x>=0);" 처럼 비교식을 감싼
//   괄호가 등장하므로, 괄호 안에서는 <expr> 전체를 허용하도록
//   일반화했다. (산술식만 있을 때는 결국 <aexp>를 파싱한 것과
//   동일하게 동작하므로 기존 문법과 충돌하지 않는다.)
// ==================================================
std::unique_ptr<Expr> Parser::factor() {
    bool negative = false;
    if (check(TokenType::MINUS)) {
        match(TokenType::MINUS);
        negative = true;
    }

    std::unique_ptr<Expr> value;

    if (check(TokenType::LPAREN)) {
        match(TokenType::LPAREN);
        value = expr();
        match(TokenType::RPAREN);
    } else if (check(TokenType::INT_LIT)) {
        Token t = match(TokenType::INT_LIT);
        value = std::make_unique<IntLiteral>(std::stoi(t.text));
    } else if (check(TokenType::ID)) {
        Token t = match(TokenType::ID);
        value = std::make_unique<Identifier>(t.text);
    } else if (check(TokenType::STR_LIT)) {
        Token t = match(TokenType::STR_LIT);
        value = std::make_unique<StrLiteral>(t.text);
    } else {
        error("number, identifier, string literal or '(' expected");
    }

    if (negative) {
        value = std::make_unique<Unary>(std::make_unique<Operator>("-"), std::move(value));
    }

    return value;
}

// <term> → <factor> { * <factor> | / <factor> }
std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> e = factor();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        std::string opText = current().text;
        pos++;
        std::unique_ptr<Expr> rhs = factor();
        e = std::make_unique<Binary>(std::make_unique<Operator>(opText), std::move(e), std::move(rhs));
    }
    return e;
}

// <aexp> → <term> { + <term> | - <term> }
std::unique_ptr<Expr> Parser::aexp() {
    std::unique_ptr<Expr> e = term();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        std::string opText = current().text;
        pos++;
        std::unique_ptr<Expr> rhs = term();
        e = std::make_unique<Binary>(std::make_unique<Operator>(opText), std::move(e), std::move(rhs));
    }
    return e;
}

bool Parser::isRelop() const {
    switch (current().type) {
        case TokenType::EQ: case TokenType::NEQ:
        case TokenType::LT: case TokenType::GT:
        case TokenType::LE: case TokenType::GE:
            return true;
        default:
            return false;
    }
}

std::string Parser::relopText() {
    std::string t = current().text;
    pos++;
    return t;
}

// <bexp> → <aexp> [<relop> <aexp>]
std::unique_ptr<Expr> Parser::bexp() {
    std::unique_ptr<Expr> left = aexp();
    if (isRelop()) {
        std::string opText = relopText();
        std::unique_ptr<Expr> right = aexp();
        return std::make_unique<Binary>(std::make_unique<Operator>(opText), std::move(left), std::move(right));
    }
    return left;
}

// <expr> → <bexp> {& <bexp> | '|' <bexp>} | !<expr> | true | false
std::unique_ptr<Expr> Parser::expr() {
    if (check(TokenType::NOT)) {
        match(TokenType::NOT);
        std::unique_ptr<Expr> e = expr();
        return std::make_unique<Unary>(std::make_unique<Operator>("!"), std::move(e));
    }
    if (check(TokenType::KW_TRUE)) {
        match(TokenType::KW_TRUE);
        return std::make_unique<BoolLiteral>(true);
    }
    if (check(TokenType::KW_FALSE)) {
        match(TokenType::KW_FALSE);
        return std::make_unique<BoolLiteral>(false);
    }

    std::unique_ptr<Expr> e = bexp();
    while (check(TokenType::AND) || check(TokenType::OR)) {
        std::string opText = current().text;
        pos++;
        std::unique_ptr<Expr> rhs = bexp();
        e = std::make_unique<Binary>(std::make_unique<Operator>(opText), std::move(e), std::move(rhs));
    }
    return e;
}

// <type> → int | bool | string
Type Parser::typeSpec() {
    if (check(TokenType::KW_INT)) { match(TokenType::KW_INT); return Type::INT; }
    if (check(TokenType::KW_BOOL)) { match(TokenType::KW_BOOL); return Type::BOOL; }
    if (check(TokenType::KW_STRING)) { match(TokenType::KW_STRING); return Type::STRING; }
    error("type (int/bool/string) expected");
    return Type::UNDEF; // 도달하지 않음
}

// <decl> → <type> id [=<expr>];
std::unique_ptr<Decl> Parser::decl() {
    Type t = typeSpec();
    Token idTok = match(TokenType::ID);
    auto id = std::make_unique<Identifier>(idTok.text);

    std::unique_ptr<Expr> init = nullptr;
    if (check(TokenType::ASSIGN)) {
        match(TokenType::ASSIGN);
        init = expr();
    }
    match(TokenType::SEMICOLON);
    return std::make_unique<Decl>(t, std::move(id), std::move(init));
}

// <stmt> → id = <expr>;
//        | '{' <stmts> '}'
//        | if (<expr>) then <stmt> [else <stmt>]
//        | while (<expr>) <stmt>
//        | read id;
//        | print <expr>;
//        | let <decls> in <stmts> end;
std::unique_ptr<Stmt> Parser::stmt() {
    if (check(TokenType::ID)) {
        Token idTok = match(TokenType::ID);
        auto id = std::make_unique<Identifier>(idTok.text);
        match(TokenType::ASSIGN);
        std::unique_ptr<Expr> e = expr();
        match(TokenType::SEMICOLON);
        return std::make_unique<Assignment>(std::move(id), std::move(e));
    }

    if (check(TokenType::LBRACE)) {
        match(TokenType::LBRACE);
        auto block = std::make_unique<Block>();
        block->stmts = stmtsList();
        match(TokenType::RBRACE);
        return block;
    }

    if (check(TokenType::KW_IF)) {
        match(TokenType::KW_IF);
        match(TokenType::LPAREN);
        std::unique_ptr<Expr> cond = expr();
        match(TokenType::RPAREN);
        match(TokenType::KW_THEN);
        std::unique_ptr<Stmt> thenS = stmt();

        auto ifNode = std::make_unique<If>();
        ifNode->cond = std::move(cond);
        ifNode->thenStmt = std::move(thenS);

        if (check(TokenType::KW_ELSE)) {
            match(TokenType::KW_ELSE);
            ifNode->elseStmt = stmt();
        }
        return ifNode;
    }

    if (check(TokenType::KW_WHILE)) {
        match(TokenType::KW_WHILE);
        match(TokenType::LPAREN);
        std::unique_ptr<Expr> cond = expr();
        match(TokenType::RPAREN);
        std::unique_ptr<Stmt> body = stmt();

        auto whileNode = std::make_unique<While>();
        whileNode->cond = std::move(cond);
        whileNode->body = std::move(body);
        return whileNode;
    }

    if (check(TokenType::KW_READ)) {
        match(TokenType::KW_READ);
        Token idTok = match(TokenType::ID);
        match(TokenType::SEMICOLON);
        return std::make_unique<Read>(std::make_unique<Identifier>(idTok.text));
    }

    if (check(TokenType::KW_PRINT)) {
        match(TokenType::KW_PRINT);
        std::unique_ptr<Expr> e = expr();
        match(TokenType::SEMICOLON);
        return std::make_unique<Print>(std::move(e));
    }

    if (check(TokenType::KW_LET)) {
        match(TokenType::KW_LET);
        auto letNode = std::make_unique<Let>();
        letNode->decls = declsList();
        match(TokenType::KW_IN);
        letNode->stmts = stmtsList();
        match(TokenType::KW_END);
        match(TokenType::SEMICOLON);
        return letNode;
    }

    error("statement expected");
    return nullptr; // 도달하지 않음
}

// <stmts> → {<stmt>}
std::vector<std::unique_ptr<Stmt>> Parser::stmtsList() {
    std::vector<std::unique_ptr<Stmt>> result;
    while (isStmtStart()) {
        result.push_back(stmt());
    }
    return result;
}

// <decls> → {<decl>}
std::vector<std::unique_ptr<Decl>> Parser::declsList() {
    std::vector<std::unique_ptr<Decl>> result;
    while (isTypeStart()) {
        result.push_back(decl());
    }
    return result;
}

// <command> → <decl> | <stmt>
std::unique_ptr<Command> Parser::command() {
    if (isTypeStart()) {
        return decl();
    }
    return stmt();
}

// <program> → {<command>}
std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    while (!check(TokenType::END_OF_INPUT)) {
        program->commands.push_back(command());
    }
    return program;
}
