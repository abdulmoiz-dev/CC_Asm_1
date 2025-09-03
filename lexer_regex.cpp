#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <regex>

using namespace std;


enum TokenType
{
    T_FUNCTION, T_INT, T_FLOAT, T_STRING, T_BOOL, T_RETURN, T_IDENTIFIER,
    T_INTLIT, T_FLOATLIT, T_STRINGLIT, T_BOOLLIT, T_PARENL, T_PARENR,
    T_BRACEL, T_BRACER, T_BRACKETL, T_BRACKETR, T_COMMA, T_SEMICOLON,
    T_COLON, T_ASSIGNOP, T_EQUALSOP, T_NOTEQUALS, T_LESSTHAN,
    T_GREATERTHAN, T_LESSEQUAL, T_GREATEREQUAL, T_PLUS, T_MINUS, T_MULT,
    T_DIV, T_AND, T_OR, T_NOT, T_IF, T_ELSE, T_WHILE, T_FOR,
    T_COMMENT, T_UNKNOWN, T_EOF, T_OUTPUTOP, T_INPUTOP
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
    vector<string> errors;
    int lineNumber = 1;

    
    unordered_map<string, TokenType> keywords = {
        {"fn", T_FUNCTION}, {"int", T_INT}, {"float", T_FLOAT},
        {"string", T_STRING}, {"bool", T_BOOL}, {"return", T_RETURN},
        {"if", T_IF}, {"else", T_ELSE}, {"while", T_WHILE},
        {"for", T_FOR}, {"true", T_BOOLLIT}, {"false", T_BOOLLIT}
    };

public:
    Lexer() = default;

    
    vector<Token> tokenize(const string &source_code)
    {
        lineNumber = 1;
        tokens.clear();
        errors.clear();

        
        
        string token_patterns[] = {
            R"(\/\/.*)",                                  
            R"(\/\*[\s\S]*?\*\/)",                        
            R"(\/\*[\s\S]*)",                             
            R"(\"(?:\\.|[^\"\\])*\")",                     
            R"(\"(?:\\.|[^\"\\])*)",                      
            R"(\b(fn|int|float|string|bool|return|if|else|while|for|true|false)\b)", 
            R"([a-zA-Z_][a-zA-Z0-9_]*)",                  
            R"(\d+\.\d+)",                                
            R"(\d+)",                                     
            R"(<<)",                                      
            R"(>>)",                                      
            R"(!=)",                                      
            R"(==)",                                      
            R"(<=)",                                      
            R"(>=)",                                      
            R"(&&)",                                      
            R"(\|\|)",                                      
            R"(\()", R"(\))", R"(\{)", R"(\})", R"(\[)", R"(\])", 
            R"(,)", R"(;)", R"(:)",                                      
            R"(\+)", R"(-)", R"(\*)", R"(\/)", R"(=)", R"(<)", R"(>)", R"(!)", 
            R"(\s+)",                                     
            R"(.)"                                        
        };

        
        string regex_str;
        for (const auto& p : token_patterns) {
            if (!regex_str.empty()) regex_str += "|";
            regex_str += "(" + p + ")";
        }

        regex master_regex(regex_str);
        auto words_begin = sregex_iterator(source_code.begin(), source_code.end(), master_regex);
        auto words_end = sregex_iterator();

        for (sregex_iterator i = words_begin; i != words_end; ++i) {
            smatch match = *i;
            string match_str = match.str();

            size_t group_idx = 0;
            for (size_t j = 1; j < match.size(); ++j) {
                if (match[j].matched) {
                    group_idx = j;
                    break;
                }
            }
            
            
            if (group_idx == 1 || group_idx == 2) { 
                
                lineNumber += count(match_str.begin(), match_str.end(), '\n');
            } else if (group_idx == 3) { 
                 errors.push_back("Error: Unclosed multi-line comment starting at line " + to_string(lineNumber));
                 lineNumber += count(match_str.begin(), match_str.end(), '\n');
            } else if (group_idx == 4) { 
                add_token(T_STRINGLIT, match_str.substr(1, match_str.length() - 2));
                lineNumber += count(match_str.begin(), match_str.end(), '\n');
            } else if (group_idx == 5) { 
                 errors.push_back("Error: Unclosed string literal starting at line " + to_string(lineNumber));
                 lineNumber += count(match_str.begin(), match_str.end(), '\n');
            } else if (group_idx == 6) { 
                add_token(keywords[match_str], match_str);
            } else if (group_idx == 7) { 
                add_token(T_IDENTIFIER, match_str);
            } else if (group_idx == 8) { 
                add_token(T_FLOATLIT, match_str);
            } else if (group_idx == 9) { 
                add_token(T_INTLIT, match_str);
            } else if (group_idx >= 10 && group_idx <= 29) { 
                
                TokenType type = T_UNKNOWN;
                if (match_str == "<<") type = T_OUTPUTOP;
                else if (match_str == ">>") type = T_INPUTOP;
                else if (match_str == "!=") type = T_NOTEQUALS;
                else if (match_str == "==") type = T_EQUALSOP;
                else if (match_str == "<=") type = T_LESSEQUAL;
                else if (match_str == ">=") type = T_GREATEREQUAL;
                else if (match_str == "&&") type = T_AND;
                else if (match_str == "||") type = T_OR;
                else if (match_str == "(") type = T_PARENL;
                else if (match_str == ")") type = T_PARENR;
                else if (match_str == "{") type = T_BRACEL;
                else if (match_str == "}") type = T_BRACER;
                else if (match_str == "[") type = T_BRACKETL;
                else if (match_str == "]") type = T_BRACKETR;
                else if (match_str == ",") type = T_COMMA;
                else if (match_str == ";") type = T_SEMICOLON;
                else if (match_str == ":") type = T_COLON;
                else if (match_str == "+") type = T_PLUS;
                else if (match_str == "-") type = T_MINUS;
                else if (match_str == "*") type = T_MULT;
                else if (match_str == "/") type = T_DIV;
                else if (match_str == "=") type = T_ASSIGNOP;
                else if (match_str == "<") type = T_LESSTHAN;
                else if (match_str == ">") type = T_GREATERTHAN;
                else if (match_str == "!") type = T_NOT;
                add_token(type, match_str);
            } else if (group_idx == 30) { 
                 lineNumber += count(match_str.begin(), match_str.end(), '\n');
            } else if (group_idx == 31) { 
                errors.push_back("Warning at line " + to_string(lineNumber) + ": Unknown character '" + match_str + "'");
                add_token(T_UNKNOWN, match_str);
            }
        }

        tokens.push_back({T_EOF, "", lineNumber});
        return tokens;
    }

    const vector<string>& getErrors() const {
        return errors;
    }

private:
    
    void add_token(TokenType type, const string& value) {
        tokens.push_back({type, value, lineNumber});
    }
};


string tokenTypeToString(TokenType type);

int main()
{
    ifstream file("test_input.txt");
    if (!file) {
        cerr << "Error: Could not open test_input.txt" << endl;
        return 1;
    }

    string source((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    Lexer lexer;
    vector<Token> tokens = lexer.tokenize(source);

    
    const auto& errors = lexer.getErrors();
    if (!errors.empty()) {
        cerr << "Lexical analysis failed with " << errors.size() << " errors:" << endl;
        for (const string& err : errors) {
            cerr << err << endl;
        }
    }
    
    
    cout << "Token stream:" << endl;
    for (const auto &token : tokens) {
        if (token.type == T_EOF) break;
        cout << "<" << tokenTypeToString(token.type) << ", \"" << token.value << "\", line " << token.line << ">" << endl;
    }

    return 0;
}


string tokenTypeToString(TokenType type) {
    static const unordered_map<TokenType, string> typeMap = {
        {T_FUNCTION, "T_FUNCTION"}, {T_INT, "T_INT"}, {T_FLOAT, "T_FLOAT"},
        {T_STRING, "T_STRING"}, {T_BOOL, "T_BOOL"}, {T_RETURN, "T_RETURN"},
        {T_IDENTIFIER, "T_IDENTIFIER"}, {T_INTLIT, "T_INTLIT"}, {T_FLOATLIT, "T_FLOATLIT"},
        {T_STRINGLIT, "T_STRINGLIT"}, {T_BOOLLIT, "T_BOOLLIT"}, {T_PARENL, "T_PARENL"},
        {T_PARENR, "T_PARENR"}, {T_BRACEL, "T_BRACEL"}, {T_BRACER, "T_BRACER"},
        {T_BRACKETL, "T_BRACKETL"}, {T_BRACKETR, "T_BRACKETR"}, {T_COMMA, "T_COMMA"},
        {T_SEMICOLON, "T_SEMICOLON"}, {T_COLON, "T_COLON"}, {T_ASSIGNOP, "T_ASSIGNOP"},
        {T_EQUALSOP, "T_EQUALSOP"}, {T_NOTEQUALS, "T_NOTEQUALS"}, {T_LESSTHAN, "T_LESSTHAN"},
        {T_GREATERTHAN, "T_GREATERTHAN"}, {T_LESSEQUAL, "T_LESSEQUAL"}, {T_GREATEREQUAL, "T_GREATEREQUAL"},
        {T_PLUS, "T_PLUS"}, {T_MINUS, "T_MINUS"}, {T_MULT, "T_MULT"}, {T_DIV, "T_DIV"},
        {T_AND, "T_AND"}, {T_OR, "T_OR"}, {T_NOT, "T_NOT"}, {T_IF, "T_IF"},
        {T_ELSE, "T_ELSE"}, {T_WHILE, "T_WHILE"}, {T_FOR, "T_FOR"}, {T_COMMENT, "T_COMMENT"},
        {T_UNKNOWN, "T_UNKNOWN"}, {T_EOF, "T_EOF"}, {T_OUTPUTOP, "T_OUTPUTOP"}, {T_INPUTOP, "T_INPUTOP"}
    };
    auto it = typeMap.find(type);
    if (it != typeMap.end()) {
        return it->second;
    }
    return "UNKNOWN_TOKEN_TYPE";
}   