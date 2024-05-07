
#include <iostream>
#include <stdexcept>
#include <string>
#include <map>

// Token stuff
struct token
{
    char kind;       // what kind of token
    double value;    // for numbers: a value
    std::string name; // for variables: name of the variable

    // constructors
    token(char ch)
            : kind(ch), value(0)
    {
    }
    token(char ch, double val)
            : kind(ch), value(val)
    {
    }
    token(char ch, std::string n)
            : kind(ch), name(n)
    {
    }
};

class token_stream
{
    bool full;       // is there a token in the buffer?
    token buffer;    // here is where we keep a Token put back using putback()
public:
    // user interface:
    token get();            // get a token
    void putback(token);    // put a token back into the token_stream

    // constructor: make a token_stream, the buffer starts empty
    token_stream()
            : full(false), buffer(0)
    {
    }
};

// single global instance of the token_stream
token_stream ts;

void token_stream::putback(token t)
{
    if (full)
        throw std::runtime_error("putback() into a full buffer");
    buffer = t;
    full = true;
}

token token_stream::get()    // read a token from the token_stream
{
    if (full)
    {
        full = false;
        return buffer;
    }

    char ch;
    std::cin >> ch;

    switch (ch)
    {
        case '(':
        case ')':
        case ';':
        case 'q':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '=':
            return token(ch);
        case '.':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        {
            std::cin.putback(ch);
            double val;
            std::cin >> val;
            return token('8', val);
        }
        default:
            if (isalpha(ch))
            {
                std::string s;
                s += ch;
                while (std::cin.get(ch) && (isalnum(ch) || ch == '_'))
                    s += ch;
                std::cin.putback(ch);
                if (s == "pi") return token('8', 3.14159265358979323846);
                return token('a', s);
            }
            throw std::runtime_error("Bad token");
    }
}

double expression();

double primary()
{
    token t = ts.get();
    switch (t.kind)
    {
        case '(':
        {
            double d = expression();
            t = ts.get();
            if (t.kind != ')')
                throw std::runtime_error("')' expected");
            return d;
        }
        case '8':
            return t.value;
        case 'a':
        {
            if (t.name == "pi") return 3.14159265358979323846;
            throw std::runtime_error("Unknown variable");
        }
        case '-':
            return -primary();
        default:
            throw std::runtime_error("primary expected");
    }
}

double term()
{
    double left = primary();
    while (true)
    {
        token t = ts.get();
        switch (t.kind)
        {
            case '*':
                left *= primary();
                break;
            case '/':
            {
                double d = primary();
                if (d == 0)
                    throw std::runtime_error("divide by zero");
                left /= d;
                break;
            }
            case '%':
            {
                double d = primary();
                if (d == 0)
                    throw std::runtime_error("divide by zero");
                left = left - d * int(left / d);
                break;
            }
            default:
                ts.putback(t);
                return left;
        }
    }
}

double expression()
{
    double left = term();
    while (true)
    {
        token t = ts.get();
        switch (t.kind)
        {
            case '+':
                left += term();
                break;
            case '-':
                left -= term();
                break;
            default:
                ts.putback(t);
                return left;
        }
    }
}

std::map<std::string, double> table;

int main()
{
    try
    {
        double val = 0;

        while (std::cin)
        {
            token t = ts.get();
            if (t.kind == 'q')
                break;

            if (t.kind == ';')
            {
                std::cout << "= " << val << '\n';
                val = 0;
            }
            else if (t.kind == '=')
            {
                token var_name = ts.get();
                if (var_name.kind != 'a')
                    throw std::runtime_error("Name expected after '='");
                token equals = ts.get();
                if (equals.kind != '=')
                    throw std::runtime_error("= missing in variable definition");
                double d = expression();
                table[var_name.name] = d;
            }
            else
            {
                ts.putback(t);
                val = expression();
            }
        }
    }
    catch (std::runtime_error& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "exception \n";
        return 2;
    }
    return 0;
}