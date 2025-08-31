# Compile the fixed regex version
g++ -std=c++11 -o lexer_regex lexer_regex.cpp

# Compile the non-regex version
g++ -std=c++11 -o lexer_noregex lexer_noregex.cpp

# Run the regex version
./lexer_regex

# Run the non-regex version
./lexer_noregex