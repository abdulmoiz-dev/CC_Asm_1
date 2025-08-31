#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <algorithm>

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
    string source;
    vector<Token> tokens;
    vector<string> errors;
    size_t pos = 0;
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
        source = source_code;
        pos = 0;
        lineNumber = 1;
        tokens.clear();
        errors.clear();

        while (!is_at_end())
        {
            scan_token();
        }

        tokens.push_back({T_EOF, "", lineNumber});
        return tokens;
    }

    const vector<string>& getErrors() const {
        return errors;
    }

private:
    bool is_at_end() {
        return pos >= source.length();
    }

    char advance() {
        return source[pos++];
    }

    char peek() {
        if (is_at_end()) return '\0';
        return source[pos];
    }

    char peek_next() {
        if (pos + 1 >= source.length()) return '\0';
        return source[pos + 1];
    }

    
    void scan_token()
    {
        char c = advance();
        switch (c)
        {
            
            case '(': add_token(T_PARENL, "("); break;
            case ')': add_token(T_PARENR, ")"); break;
            case '{': add_token(T_BRACEL, "{"); break;
            case '}': add_token(T_BRACER, "}"); break;
            case '[': add_token(T_BRACKETL, "["); break;
            case ']': add_token(T_BRACKETR, "]"); break;
            case ',': add_token(T_COMMA, ","); break;
            case ';': add_token(T_SEMICOLON, ";"); break;
            case ':': add_token(T_COLON, ":"); break;
            case '+': add_token(T_PLUS, "+"); break;
            case '-': add_token(T_MINUS, "-"); break;
            case '*': add_token(T_MULT, "*"); break;

            
            case '!': add_token(match('=') ? T_NOTEQUALS : T_NOT); break;
            case '=': add_token(match('=') ? T_EQUALSOP : T_ASSIGNOP); break;
            case '<': add_token(match('=') ? T_LESSEQUAL : (match('<') ? T_OUTPUTOP : T_LESSTHAN)); break;
            case '>': add_token(match('=') ? T_GREATEREQUAL : (match('>') ? T_INPUTOP : T_GREATERTHAN)); break;

            case '&': if (match('&')) { add_token(T_AND); } else { unknown_character(c); } break;
            case '|': if (match('|')) { add_token(T_OR); } else { unknown_character(c); } break;

            
            case '/':
                if (match('/')) {
                    
                    while (peek() != '\n' && !is_at_end()) advance();
                } else if (match('*')) {
                    
                    handle_multiline_comment();
                } else {
                    add_token(T_DIV, "/");
                }
                break;

            
            case ' ':
            case '\r':
            case '\t':
                
                break;
            case '\n':
                lineNumber++;
                break;
            
            
            case '"': handle_string(); break;

            default:
                if (isdigit(c)) {
                    handle_number();
                } else if (isalpha(c) || c == '_') {
                    handle_identifier();
                } else {
                    unknown_character(c);
                }
                break;
        }
    }


    void handle_multiline_comment() {
        size_t startLine = lineNumber;
        while (!(peek() == '*' && peek_next() == '/') && !is_at_end()) {
            if (peek() == '\n') lineNumber++;
            advance();
        }

        if (is_at_end()) {
            
            errors.push_back("Error: Unclosed multi-line comment starting at line " + to_string(startLine));
            return;
        }

        
        advance();
        advance();
    }

    void handle_string() {
        size_t startLine = lineNumber;
        string value;
        while (peek() != '"' && !is_at_end()) {
            if (peek() == '\n') lineNumber++;
            
            if (peek() == '\\' && !is_at_end()) {
                advance(); 
                switch (peek()) {
                    case 'n': value += '\n'; break;
                    case 't': value += '\t'; break;
                    case '"': value += '"'; break;
                    case '\\': value += '\\'; break;
                    default: value += peek(); break;
                }
            } else {
                value += peek();
            }
            advance();
        }

        if (is_at_end()) {
            
            errors.push_back("Error: Unclosed string literal starting at line " + to_string(startLine));
            return;
        }

        
        advance();
        add_token(T_STRINGLIT, value);
    }
    
    void handle_number() {
        size_t start = pos - 1;
        while (isdigit(peek())) advance();

        
        if (peek() == '.' && isdigit(peek_next())) {
            
            advance();
            while (isdigit(peek())) advance();
            add_token(T_FLOATLIT, source.substr(start, pos - start));
        } else {
            add_token(T_INTLIT, source.substr(start, pos - start));
        }

        
        if (isalpha(peek()) || peek() == '_') {
            errors.push_back("Error at line " + to_string(lineNumber) + ": Invalid identifier. Identifiers cannot start with a number.");
            
            while (isalnum(peek()) || peek() == '_') advance();
        }
    }

    void handle_identifier() {
        size_t start = pos - 1;
        while (isalnum(peek()) || peek() == '_') advance();

        string text = source.substr(start, pos - start);
        TokenType type;
        if (keywords.count(text)) {
            type = keywords[text];
        } else {
            type = T_IDENTIFIER;
        }
        add_token(type, text);
    }

    void unknown_character(char c) {
        
        errors.push_back("Warning at line " + to_string(lineNumber) + ": Unknown character '" + c + "'");
        add_token(T_UNKNOWN, string(1, c));
    }


    bool match(char expected) {
        if (is_at_end()) return false;
        if (source[pos] != expected) return false;
        pos++;
        return true;
    }

    void add_token(TokenType type) {
        add_token(type, "");
    }

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
        return 1; 
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