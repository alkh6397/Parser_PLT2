#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// ==================================================
// 들여쓰기 출력을 담당하는 헬퍼
// ==================================================
class Indent {
public:
    static void display(int level, const std::string& s) {
        std::cout << std::endl;
        for (int i = 0; i < level; i++) std::cout << "  ";
        std::cout << s;
    }
};

enum class Type { INT, BOOL, STRING, UNDEF };

inline std::string typeToString(Type t) {
    switch (t) {
        case Type::INT: return "int";
        case Type::BOOL: return "bool";
        case Type::STRING: return "string";
        default: return "undef";
    }
}

// ==================================================
// 모든 AST 노드의 최상위 클래스
// Command = Decl | Stmt   (참고자료의 클래스 체계를 따름)
// ==================================================
class Command {
public:
    virtual ~Command() = default;
    virtual void display(int level) = 0;
};

// 수식(expr)에 해당하는 노드들의 베이스
class Expr : public Command {
public:
    virtual ~Expr() = default;
};

// 문장(stmt)에 해당하는 노드들의 베이스
class Stmt : public Command {
public:
    virtual ~Stmt() = default;
};

// --------------------------------------------------
// 연산자 노드 (Binary/Unary가 들고 있는 부속 정보)
// --------------------------------------------------
class Operator {
public:
    std::string op;
    explicit Operator(const std::string& o) : op(o) {}
    void display(int level) {
        Indent::display(level, "Operator: " + op);
    }
};

// ==================================================
// Expr 계열 노드
// ==================================================

class Identifier : public Expr {
public:
    std::string name;
    explicit Identifier(const std::string& n) : name(n) {}
    void display(int level) override {
        Indent::display(level, "Identifier: " + name);
    }
};

class IntLiteral : public Expr {
public:
    int value;
    explicit IntLiteral(int v) : value(v) {}
    void display(int level) override {
        Indent::display(level, "Value: " + std::to_string(value));
    }
};

class BoolLiteral : public Expr {
public:
    bool value;
    explicit BoolLiteral(bool v) : value(v) {}
    void display(int level) override {
        Indent::display(level, std::string("Value: ") + (value ? "true" : "false"));
    }
};

class StrLiteral : public Expr {
public:
    std::string value;
    explicit StrLiteral(const std::string& v) : value(v) {}
    void display(int level) override {
        Indent::display(level, "Value: \"" + value + "\"");
    }
};

// 이항 연산 (+ - * / == != < > <= >= & |)
class Binary : public Expr {
public:
    std::unique_ptr<Operator> op;
    std::unique_ptr<Expr> expr1, expr2;
    Binary(std::unique_ptr<Operator> o, std::unique_ptr<Expr> e1, std::unique_ptr<Expr> e2)
        : op(std::move(o)), expr1(std::move(e1)), expr2(std::move(e2)) {}
    void display(int level) override {
        Indent::display(level, "Binary");
        op->display(level + 1);
        expr1->display(level + 1);
        expr2->display(level + 1);
    }
};

// 단항 연산 (음수 -, 논리부정 !)
class Unary : public Expr {
public:
    std::unique_ptr<Operator> op;
    std::unique_ptr<Expr> expr;
    Unary(std::unique_ptr<Operator> o, std::unique_ptr<Expr> e)
        : op(std::move(o)), expr(std::move(e)) {}
    void display(int level) override {
        Indent::display(level, "Unary");
        op->display(level + 1);
        expr->display(level + 1);
    }
};

// ==================================================
// Decl : <type> id [= <expr>];
// ==================================================
class Decl : public Command {
public:
    Type type;
    std::unique_ptr<Identifier> id;
    std::unique_ptr<Expr> init; // nullptr이면 초기값 없음
    Decl(Type t, std::unique_ptr<Identifier> i, std::unique_ptr<Expr> e)
        : type(t), id(std::move(i)), init(std::move(e)) {}
    void display(int level) override {
        Indent::display(level, "Decl");
        Indent::display(level + 1, "Type: " + typeToString(type));
        id->display(level + 1);
        if (init) init->display(level + 1);
    }
};

// ==================================================
// Stmt 계열 노드
// ==================================================

// id = <expr>;
class Assignment : public Stmt {
public:
    std::unique_ptr<Identifier> id;
    std::unique_ptr<Expr> expr;
    Assignment(std::unique_ptr<Identifier> i, std::unique_ptr<Expr> e)
        : id(std::move(i)), expr(std::move(e)) {}
    void display(int level) override {
        Indent::display(level, "Assignment");
        id->display(level + 1);
        expr->display(level + 1);
    }
};

// '{' <stmts> '}'
class Block : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> stmts;
    void display(int level) override {
        Indent::display(level, "Block");
        for (auto& s : stmts) s->display(level + 1);
    }
};

// if (<expr>) then <stmt> [else <stmt>]
class If : public Stmt {
public:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> thenStmt;
    std::unique_ptr<Stmt> elseStmt; // nullptr이면 else 없음
    void display(int level) override {
        Indent::display(level, "If");
        Indent::display(level + 1, "Cond");
        cond->display(level + 2);
        Indent::display(level + 1, "Then");
        thenStmt->display(level + 2);
        if (elseStmt) {
            Indent::display(level + 1, "Else");
            elseStmt->display(level + 2);
        }
    }
};

// while (<expr>) <stmt>
class While : public Stmt {
public:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stmt> body;
    void display(int level) override {
        Indent::display(level, "While");
        Indent::display(level + 1, "Cond");
        cond->display(level + 2);
        Indent::display(level + 1, "Body");
        body->display(level + 2);
    }
};

// read id;
class Read : public Stmt {
public:
    std::unique_ptr<Identifier> id;
    explicit Read(std::unique_ptr<Identifier> i) : id(std::move(i)) {}
    void display(int level) override {
        Indent::display(level, "Read");
        id->display(level + 1);
    }
};

// print <expr>;
class Print : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit Print(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void display(int level) override {
        Indent::display(level, "Print");
        expr->display(level + 1);
    }
};

// let <decls> in <stmts> end;
class Let : public Stmt {
public:
    std::vector<std::unique_ptr<Decl>> decls;
    std::vector<std::unique_ptr<Stmt>> stmts;
    void display(int level) override {
        Indent::display(level, "Let");
        Indent::display(level + 1, "Decls");
        for (auto& d : decls) d->display(level + 2);
        Indent::display(level + 1, "Stmts");
        for (auto& s : stmts) s->display(level + 2);
    }
};

// ==================================================
// Program : {<command>}  — 전체 프로그램(최상위 노드)
// ==================================================
class Program {
public:
    std::vector<std::unique_ptr<Command>> commands;
    void display() {
        std::cout << "Program";
        for (auto& c : commands) c->display(1);
        std::cout << std::endl;
    }
};

#endif
