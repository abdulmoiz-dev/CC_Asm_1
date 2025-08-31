# Compile the fixed regex version
g++ -std=c++11 -o lexer_regex lexer_regex.cpp

# Compile the non-regex version
g++ -std=c++11 -o lexer_noregex lexer_noregex.cpp

# Run the regex version
./lexer_regex

# Run the non-regex version
BONUS:
run this in cmd,

chcp 65001

to run in UTF-8 mode for Unicode character support e.g emojis or non-latin characters for identifier names, string literals, etc.


Run command:
./lexer_noregex
