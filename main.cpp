#include <iostream>
#include <string>
#include "Lexer.h"
#include "Parser.h"
#include "AST.h"

// 소스코드 문자열을 파싱해서 AST를 트리 형태로 출력
static void runAndDisplay(const std::string& title, const std::string& source) {
    std::cout << "==================================================\n";
    std::cout << title << "\n";
    std::cout << "==================================================\n";
    std::cout << "[input program]\n" << source << std::endl;
    std::cout << "[AST print]";

    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        std::unique_ptr<Program> program = parser.parseProgram();

        program->display();
    } catch (const std::exception& e) {
        std::cout << std::endl << "Error: " << e.what() << std::endl;
    }

    std::cout << std::endl << std::endl;
}

// ==================================================
// 입력 1: 변수 선언과 산술연산
// ==================================================
void testCase1() {
    std::string source =
        "int x = 10;"
        "bool flag = true;"
        "string msg = \"hello\";"
        "x = -(x + 2) * 3 / 4 - 1;"
        "flag = (x>=0);"
        "print flag;"
        "print msg;";
    runAndDisplay("input1", source);
}

// ==================================================
// 입력 2: 복합문, if, while, read, 비교·논리연산
// ==================================================
void testCase2() {
    std::string source =
        "int x = 0;"
        "read x;"
        "if (x >= 0 & x <= 100) then "
        "print x;"
        "else "
        "print -x;"
        "while (x > 0 | x == 5) {"
        "x = x - 1;"
        "if (x != 3) then print \"loop\";"
        "}";
    runAndDisplay("input2", source);
}

// ==================================================
// 입력 3: let 문과 중첩 블록
// ==================================================
void testCase3() {
    std::string source =
        "int i = 0; "
        "int sum = 0; "
        "bool done = false; "
        "string s = \"result\"; "
        "while (i < 5) {"
        "sum = sum + i * 2;"
        "i = i + 1;"
        "}"
        "if (!(sum > 20)) then "
        "done = true;"
        "else {"
        "let int t = sum / 2; in "
        "print t;"
        "end;"
        "}"
        "print s;";
    runAndDisplay("input3", source);
}

int main() {
    testCase1();
    testCase2();
    testCase3();
    return 0;
}
