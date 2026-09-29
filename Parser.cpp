#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

using namespace std;


// ==================================================
// 계산 결과를 저장하는 구조체
// ==================================================

struct Value
{
    bool isBool;

    int intValue;
    bool boolValue;

    // 정수 값 생성
    static Value makeInt(int value)
    {
        Value result;
        result.isBool = false;
        result.intValue = value;
        result.boolValue = false;
        return result;
    }

    // 논리 값 생성
    static Value makeBool(bool value)
    {
        Value result;
        result.isBool = true;
        result.intValue = 0;
        result.boolValue = value;
        return result;
    }

    // 0은 false로, 0이 아닌 것은 true 사용하던 기존의 논리
    // 논리식에서 사용할 bool 값으로 변환
    bool asBool() const
    {
        if (isBool)
            return boolValue;

        return intValue != 0;
    }
};


// ==================================================
// Parser 클래스
// ==================================================

class Parser
{
private:

    string input;
    size_t pos;


    // ----------------------------------------------
    // 공백 건너뛰기
    // ----------------------------------------------
    void skipWhitespace()
    {
        while (pos < input.length() &&
            isspace(input[pos]))
        {
            pos++;
        }
    }


    // ----------------------------------------------
    // 현재 위치의 문자열이 str과 같은지 확인
    // 같으면 해당 문자열만큼 이동
    // ----------------------------------------------
    bool match(const string& str)
    {
        skipWhitespace();

        if (input.compare(pos, str.length(), str) == 0)
        {
            pos += str.length();
            return true;
        }

        return false;
    }


    // ----------------------------------------------
    // 오류 발생
    // ----------------------------------------------
    void error(const string& message)
    {
        throw runtime_error(
            "Syntax error at position " +
            to_string(pos) + ": " +
            message
        );
    }


    // ==================================================
    // <number> → <digit> {<digit>}
    // ==================================================

    Value number()
    {
        skipWhitespace();

        if (pos >= input.length() ||
            !isdigit(input[pos]))
        {
            error("number expected");
        }

        int value = 0;

        while (pos < input.length() &&
            isdigit(input[pos]))
        {
            value = value * 10 + (input[pos] - '0');
            pos++;
        }

        return Value::makeInt(value);
    }


    // ==================================================
    // <factor> → [-] ( <number> | (<aexp>) )
    //
    // 실제 입력 예제인 !(3>5) 등을 처리하기 위해
    // 괄호 안에는 <expr>도 허용
    // ==================================================

    Value factor()
    {
        skipWhitespace();

        bool negative = false;

        // [-]
        if (match("-"))
        {
            negative = true;
        }


        Value value;


        // '('
        if (match("("))
        {
            // 여기서 알아서 계싼해 줌
            value = expr();

            // ')'
            if (!match(")"))
            {
                error("')' expected");
            }
        }
        else
        {
            // <number>
            value = number();
        }


        // 음수는 정수에 대해서만 적용
        if (negative)
        {
            if (value.isBool)
            {
                error("negative operator cannot be applied to boolean");
            }

            value.intValue = -value.intValue;
        }


        return value;
    }


    // ==================================================
    // <term> → <factor> { * <factor> | / <factor> }
    // ==================================================

    Value term()
    {
        Value value = factor();


        while (true)
        {
            // *
            if (match("*"))
            {
                Value right = factor();

                if (value.isBool || right.isBool)
                {
                    error("arithmetic operation requires numbers");
                }

                value.intValue *= right.intValue;
            }


            // /
            else if (match("/"))
            {
                Value right = factor();

                if (value.isBool || right.isBool)
                {
                    error("arithmetic operation requires numbers");
                }

                if (right.intValue == 0)
                {
                    error("division by zero");
                }

                value.intValue /= right.intValue;
            }


            else
            {
                break;
            }
        }


        return value;
    }


    // ==================================================
    // <aexp> → <term> { + <term> | - <term> }
    // ==================================================

    Value aexp()
    {
        Value value = term();


        while (true)
        {
            // +
            if (match("+"))
            {
                Value right = term();

                if (value.isBool || right.isBool)
                {
                    error("arithmetic operation requires numbers");
                }

                value.intValue += right.intValue;
            }


            // -
            else if (match("-"))
            {
                Value right = term();

                if (value.isBool || right.isBool)
                {
                    error("arithmetic operation requires numbers");
                }

                value.intValue -= right.intValue;
            }


            else
            {
                break;
            }
        }


        return value;
    }


    // ==================================================
    // <relop> → == | != | < | > | <= | >=
    // ==================================================

    string relop()
    {
        if (match("=="))
            return "==";

        if (match("!="))
            return "!=";

        if (match("<="))
            return "<=";

        if (match(">="))
            return ">=";

        if (match("<"))
            return "<";

        if (match(">"))
            return ">";

        return "";
    }


        // ==================================================
        // <bexp> → <aexp> [<relop> <aexp>]
        //
        // true / false도 논리식의 피연산자로 사용할 수 있도록
        // 확장하여 처리
        // ==================================================

        Value bexp()
    {
        skipWhitespace();

        // true
        if (match("true"))
        {
            return Value::makeBool(true);
        }

        // false
        if (match("false"))
        {
            return Value::makeBool(false);
        }

        // <aexp>
        Value left = aexp();

        // <relop>
        string op = relop();

        // 비교 연산자가 없는 경우
        if (op.empty())
        {
            return left;
        }

        // <aexp>
        Value right = aexp();

        if (left.isBool || right.isBool)
        {
            error("relational operation requires numbers");
        }

        bool result = false;

        if (op == "==")
        {
            result = left.intValue == right.intValue;
        }
        else if (op == "!=")
        {
            result = left.intValue != right.intValue;
        }
        else if (op == "<")
        {
            result = left.intValue < right.intValue;
        }
        else if (op == ">")
        {
            result = left.intValue > right.intValue;
        }
        else if (op == "<=")
        {
            result = left.intValue <= right.intValue;
        }
        else if (op == ">=")
        {
            result = left.intValue >= right.intValue;
        }

        return Value::makeBool(result);
    }


    // ==================================================
    // <expr> →
    //
    // <bexp> { & <bexp> | | <bexp> }
    // | !<expr>
    // | true
    // | false
    // ==================================================

    Value expr()
    {
        skipWhitespace();


        // !<expr>
        if (match("!"))
        {
            Value value = expr();

            return Value::makeBool(!value.asBool());
        }


        // true
        if (match("true"))
        {
            return Value::makeBool(true);
        }


        // false
        if (match("false"))
        {
            return Value::makeBool(false);
        }


        // <bexp>
        Value value = bexp();


        while (true)
        {
            // & <bexp>
            if (match("&"))
            {
                Value right = bexp();

                value = Value::makeBool(
                    value.asBool() && right.asBool()
                );
            }


            // | <bexp>
            else if (match("|"))
            {
                Value right = bexp();

                value = Value::makeBool(
                    value.asBool() || right.asBool()
                );
            }


            else
            {
                break;
            }
        }


        return value;
    }


public:

    // ==================================================
    // 생성자
    // ==================================================

    Parser(const string& str)
    {
        input = str;
        pos = 0;
    }


    // ==================================================
    // 전체 입력 파싱
    // ==================================================

    Value parse()
    {
        skipWhitespace();

        Value result = expr();

        skipWhitespace();


        // 입력이 모두 처리되었는지 확인
        if (pos != input.length())
        {
            error("unexpected character");
        }


        return result;
    }
};


// ==================================================
// 결과 출력 함수
// ==================================================

void printValue(const Value& value)
{
    if (value.isBool)
    {
        if (value.boolValue)
            cout << "true";
        else
            cout << "false";
    }
    else
    {
        cout << value.intValue;
    }
}


// ==================================================
// main
// ==================================================

int main()
{
    string input;

    cout << "Parser / Calculator" << endl;
    cout << "===================" << endl;

    cout << "입력식: " << endl;


    while (true)
    {
        cout << "\n> ";

        getline(cin, input);


        // exit 입력 시 종료
        if (input == "exit")
        {
            break;
        }


        try
        {
            Parser parser(input);

            Value result = parser.parse();


            cout << "결과: ";
            printValue(result);
            cout << endl;
        }
        catch (const exception& e)
        {
            cout << "Error: " << e.what() << endl;
        }
    }


    return 0;
}