#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <cctype>
#include <unordered_map>
#include <algorithm>

using namespace std;

enum TokenType
{
    T_FUNCTION,
    T_INT,
    T_FLOAT,
    T_STRING,
    T_BOOL,
    T_RETURN,
    T_IDENTIFIER,
    T_INTLIT,
    T_FLOATLIT,
    T_STRINGLIT,
    T_BOOLLIT,
    T_PARENL,
    T_PARENR,
    T_BRACEL,
    T_BRACER,
    T_BRACKETL,
    T_BRACKETR,
    T_COMMA,
    T_SEMICOLON,
    T_COLON,
    T_QUOTES,
    T_ASSIGNOP,
    T_EQUALSOP,
    T_NOTEQUALS,
    T_LESSTHAN,
    T_GREATERTHAN,
    T_LESSEQUAL,
    T_GREATEREQUAL,
    T_PLUS,
    T_MINUS,
    T_MULT,
    T_DIV,
    T_AND,
    T_OR,
    T_NOT,
    T_IF,
    T_ELSE,
    T_WHILE,
    T_FOR,
    T_COMMENT,
    T_UNKNOWN,
    T_EOF,
    T_OUTPUTOP,
    T_INPUTOP
};

struct Token
{
    TokenType type;
    string value;
    int line;
};

class Lexer
{
private:
    vector<Token> tokens;
    int lineNumber;

    unordered_map<string, TokenType> keywords = {
        {"fn", T_FUNCTION}, {"int", T_INT}, {"float", T_FLOAT}, {"string", T_STRING}, {"bool", T_BOOL}, {"return", T_RETURN}, {"if", T_IF}, {"else", T_ELSE}, {"while", T_WHILE}, {"for", T_FOR}, {"true", T_BOOLLIT}, {"false", T_BOOLLIT}};

    unordered_map<string, TokenType> operators = {
        {"=", T_ASSIGNOP}, {"==", T_EQUALSOP}, {"!=", T_NOTEQUALS}, {"<", T_LESSTHAN}, {">", T_GREATERTHAN}, {"<=", T_LESSEQUAL}, {">=", T_GREATEREQUAL}, {"+", T_PLUS}, {"-", T_MINUS}, {"*", T_MULT}, {"/", T_DIV}, {"&&", T_AND}, {"||", T_OR}, {"!", T_NOT}, {"<<", T_OUTPUTOP}, {">>", T_INPUTOP} // Add these
    };

public:
    Lexer() : lineNumber(1) {}

    vector<Token> tokenize(const string &source)
    {
        string remaining = source;
        tokens.clear();
        lineNumber = 1;

        while (!remaining.empty())
        {
            if (isspace(remaining[0]))
            {
                if (remaining[0] == '\n')
                    lineNumber++;
                remaining = remaining.substr(1);
                continue;
            }

            if (remaining.size() >= 2 && remaining.substr(0, 2) == "//")
            {
                size_t end = remaining.find('\n');
                if (end == string::npos)
                    break;
                remaining = remaining.substr(end);
                continue;
            }

            if (remaining.size() >= 2 && remaining.substr(0, 2) == "/*")
            {
                size_t end = remaining.find("*/");
                if (end == string::npos)
                {
                    cerr << "Error: Unclosed comment at line " << lineNumber << endl;
                    break;
                }

                string commentPart = remaining.substr(0, end + 2);
                int newlines = count(commentPart.begin(), commentPart.end(), '\n');
                lineNumber += newlines;
                remaining = remaining.substr(end + 2);
                continue;
            }

            bool matched = false;

            regex identRegex("^[a-zA-Z_][a-zA-Z0-9_]*");
            smatch match;
            if (regex_search(remaining, match, identRegex))
            {
                string ident = match.str();
                if (keywords.find(ident) != keywords.end())
                {
                    tokens.push_back({keywords[ident], ident, lineNumber});
                }
                else
                {
                    tokens.push_back({T_IDENTIFIER, ident, lineNumber});
                }
                remaining = remaining.substr(ident.length());
                matched = true;
            }

            else if (regex_search(remaining, match, regex("^\\d+\\.\\d+")))
            {
                tokens.push_back({T_FLOATLIT, match.str(), lineNumber});
                remaining = remaining.substr(match.str().length());
                matched = true;
            }
            else if (regex_search(remaining, match, regex("^\\d+")))
            {
                tokens.push_back({T_INTLIT, match.str(), lineNumber});
                remaining = remaining.substr(match.str().length());
                matched = true;
            }

            else if (remaining[0] == '"')
            {
                size_t end = 1;
                bool escaped = false;
                while (end < remaining.length())
                {
                    if (remaining[end] == '\\')
                    {
                        escaped = !escaped;
                    }
                    else if (remaining[end] == '"' && !escaped)
                    {
                        break;
                    }
                    else
                    {
                        escaped = false;
                    }
                    end++;
                }

                if (end >= remaining.length())
                {
                    cerr << "Error: Unclosed string literal at line " << lineNumber << endl;
                    break;
                }

                string strLit = remaining.substr(1, end - 1);
                tokens.push_back({T_STRINGLIT, strLit, lineNumber});
                remaining = remaining.substr(end + 1);
                matched = true;
            }

            else
            {

                bool opMatched = false;
                if (remaining.size() >= 2)
                {
                    string twoCharOp = remaining.substr(0, 2);
                    if (operators.find(twoCharOp) != operators.end())
                    {
                        tokens.push_back({operators[twoCharOp], twoCharOp, lineNumber});
                        remaining = remaining.substr(2);
                        opMatched = true;
                        matched = true;
                    }
                }

                if (!opMatched)
                {
                    string singleCharOp = string(1, remaining[0]);
                    if (operators.find(singleCharOp) != operators.end())
                    {
                        tokens.push_back({operators[singleCharOp], singleCharOp, lineNumber});
                        remaining = remaining.substr(1);
                        matched = true;
                    }
                }

                if (!matched)
                {
                    switch (remaining[0])
                    {
                    case '(':
                        tokens.push_back({T_PARENL, "(", lineNumber});
                        break;
                    case ')':
                        tokens.push_back({T_PARENR, ")", lineNumber});
                        break;
                    case '{':
                        tokens.push_back({T_BRACEL, "{", lineNumber});
                        break;
                    case '}':
                        tokens.push_back({T_BRACER, "}", lineNumber});
                        break;
                    case '[':
                        tokens.push_back({T_BRACKETL, "[", lineNumber});
                        break;
                    case ']':
                        tokens.push_back({T_BRACKETR, "]", lineNumber});
                        break;
                    case ',':
                        tokens.push_back({T_COMMA, ",", lineNumber});
                        break;
                    case ';':
                        tokens.push_back({T_SEMICOLON, ";", lineNumber});
                        break;
                    case ':':
                        tokens.push_back({T_COLON, ":", lineNumber});
                        break;
                    default:
                        tokens.push_back({T_UNKNOWN, string(1, remaining[0]), lineNumber});
                        cerr << "Warning: Unknown character '" << remaining[0]
                             << "' at line " << lineNumber << endl;
                    }
                    remaining = remaining.substr(1);
                    matched = true;
                }
            }
        }

        tokens.push_back({T_EOF, "", lineNumber});
        return tokens;
    }
};

string tokenTypeToString(TokenType type)
{
    switch (type)
    {
    case T_FUNCTION:
        return "T_FUNCTION";
    case T_INT:
        return "T_INT";
    case T_FLOAT:
        return "T_FLOAT";
    case T_STRING:
        return "T_STRING";
    case T_BOOL:
        return "T_BOOL";
    case T_RETURN:
        return "T_RETURN";
    case T_IDENTIFIER:
        return "T_IDENTIFIER";
    case T_INTLIT:
        return "T_INTLIT";
    case T_FLOATLIT:
        return "T_FLOATLIT";
    case T_STRINGLIT:
        return "T_STRINGLIT";
    case T_BOOLLIT:
        return "T_BOOLLIT";
    case T_PARENL:
        return "T_PARENL";
    case T_PARENR:
        return "T_PARENR";
    case T_BRACEL:
        return "T_BRACEL";
    case T_BRACER:
        return "T_BRACER";
    case T_BRACKETL:
        return "T_BRACKETL";
    case T_BRACKETR:
        return "T_BRACKETR";
    case T_COMMA:
        return "T_COMMA";
    case T_SEMICOLON:
        return "T_SEMICOLON";
    case T_COLON:
        return "T_COLON";
    case T_QUOTES:
        return "T_QUOTES";
    case T_ASSIGNOP:
        return "T_ASSIGNOP";
    case T_EQUALSOP:
        return "T_EQUALSOP";
    case T_NOTEQUALS:
        return "T_NOTEQUALS";
    case T_LESSTHAN:
        return "T_LESSTHAN";
    case T_GREATERTHAN:
        return "T_GREATERTHAN";
    case T_LESSEQUAL:
        return "T_LESSEQUAL";
    case T_GREATEREQUAL:
        return "T_GREATEREQUAL";
    case T_PLUS:
        return "T_PLUS";
    case T_MINUS:
        return "T_MINUS";
    case T_MULT:
        return "T_MULT";
    case T_DIV:
        return "T_DIV";
    case T_AND:
        return "T_AND";
    case T_OR:
        return "T_OR";
    case T_NOT:
        return "T_NOT";
    case T_IF:
        return "T_IF";
    case T_ELSE:
        return "T_ELSE";
    case T_WHILE:
        return "T_WHILE";
    case T_FOR:
        return "T_FOR";
    case T_COMMENT:
        return "T_COMMENT";
    case T_UNKNOWN:
        return "T_UNKNOWN";
    case T_EOF:
        return "T_EOF";
    case T_OUTPUTOP:
        return "T_OUTPUTOP";
    case T_INPUTOP:
        return "T_INPUTOP";
    default:
        return "UNKNOWN";
    }
}

int main()
{
    ifstream file("test_input.txt");
    if (!file)
    {
        cerr << "Error: Could not open test_input.txt" << endl;
        return 1;
    }

    string source((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    Lexer lexer;
    vector<Token> tokens = lexer.tokenize(source);

    cout << "Token stream:" << endl;
    for (const auto &token : tokens)
    {
        if (token.type == T_STRINGLIT)
        {
            cout << tokenTypeToString(token.type) << "(\"" << token.value << "\") ";
        }
        else if (token.type == T_IDENTIFIER || token.type == T_INTLIT ||
                 token.type == T_FLOATLIT || token.type == T_BOOLLIT)
        {
            cout << tokenTypeToString(token.type) << "(" << token.value << ") ";
        }
        else if (token.type != T_EOF)
        {
            cout << tokenTypeToString(token.type) << " ";
        }
    }
    cout << endl;

    return 0;
}