#include <iostream>
#include <string>
#include <vector>
#include <regex>

using namespace std;

enum TokenType : int {
    IDENTIFIER = 0,
    NUMBER,
    NUMBER_TYPE,
    STRING,
    STRING_TYPE,
    BOOL,
    BOOL_TYPE,
    VAR,
    CONST,
    IS,
    NULL_TYPE,
    COMMA,
    SEMICOLON,
    ASSIGN,
    PLUS,
    MINUS,
    MUL,
    DIV,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    EOF_,
    READ_INT,
    READ_REAL,
    READ_STRING,
    PRINT,
    IF,
    ELSE,
    THEN,
    END,
    LESS, 
    LESS_E,
    GREATER,
    GREATER_E,
    EQUAL,
    NOT_EQUAL, 
    NOT,
    WHILE_L,
    FOR_L,
    LOOP,
    FUNC,
    RETURN, 
    DO, 
    DOT_OP,
    INVALID,
    AND,
    OR,
    XOR,
};

const string TokenTypeStr[] = {
    [TokenType::IDENTIFIER] = "IDENTIFIER",
    [TokenType::NUMBER] = "NUMBER",
    [TokenType::NUMBER_TYPE] = "NUMBER_TYPE",
    [TokenType::STRING] = "STRING",
    [TokenType::STRING_TYPE] = "STRING_TYPE",
    [TokenType::BOOL] = "BOOL",
    [TokenType::BOOL_TYPE] = "BOOL_TYPE",
    [TokenType::VAR] = "VAR",
    [TokenType::CONST] = "CONST",
    [TokenType::IS] = "IS",
    [TokenType::NULL_TYPE] = "NULL_TYPE",
    [TokenType::COMMA] = "COMMA",
    [TokenType::SEMICOLON] = "SEMICOLON",
    [TokenType::ASSIGN] = "ASSIGN",
    [TokenType::PLUS] = "PLUS",
    [TokenType::MINUS] = "MINUS",
    [TokenType::MUL] = "MUL",
    [TokenType::DIV] = "DIV",
    [TokenType::LPAREN] = "LPAREN",
    [TokenType::RPAREN] = "RPAREN",
    [TokenType::LBRACKET] = "LBRACKET",
    [TokenType::RBRACKET] = "RBRACKET",
    [TokenType::LBRACE] = "LBRACE",
    [TokenType::RBRACE] = "RBRACE",
    [TokenType::EOF_] = "EOF_",
    [TokenType::READ_INT] = "READ_INT",
    [TokenType::READ_REAL] = "READ_REAL",
    [TokenType::READ_STRING] = "READ_STRING",
    [TokenType::PRINT] = "PRINT",
    [TokenType::IF] = "IF",
    [TokenType::ELSE] = "ELSE",
    [TokenType::THEN] = "THEN",
    [TokenType::END] = "END",
    [TokenType::LESS] = "LESS",
    [TokenType::LESS_E] = "LESS_E",
    [TokenType::GREATER] = "GREATER",
    [TokenType::GREATER_E] = "GREATER_E",
    [TokenType::EQUAL] = "EQUAL",
    [TokenType::NOT_EQUAL] = "NOT_EQUAL",
    [TokenType::NOT] = "NOT",
    [TokenType::WHILE_L] = "WHILE_L",
    [TokenType::FOR_L] = "FOR_L",
    [TokenType::LOOP] = "LOOP",
    [TokenType::FUNC] = "FUNC",
    [TokenType::RETURN] = "RETURN",
    [TokenType::DO] = "DO",
    [TokenType::DOT_OP] = "DOT_OP",
    [TokenType::INVALID] = "INVALID",
    [TokenType::AND] = "AND",
    [TokenType::OR] = "OR",
    [TokenType::XOR] = "XOR",
};

class Token {
    public:
        Token(TokenType type, string lexeme, size_t line) : type(type), lexeme(lexeme), line(line) {}

        Token(TokenType type, string lexeme) : type(type), lexeme(lexeme) {}

        TokenType getType() const { return type; }

        string getLexeme() const { return lexeme; }

        size_t getLine() const { return line; }

    private:
        TokenType type;
        string lexeme;
        size_t line;
};

class Lexer {
    public:
        Lexer(string input) : input(input) {}

        vector<Token> tokenize() {
            vector<Token> tokens;
            size_t i = 0;
            size_t curentLine = 0;
            regex pattern("[a-zA-Z_]");

            while (i < input.length()) {
                char c = input[i];

                if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                    if(c == '\n') curentLine++;
                    i++;
                } else if (regex_match(string(1, c), pattern)) {
                    string identifier = readIdentifier(i);
                    TokenType type = getType(identifier);
                    tokens.emplace_back(type, identifier, curentLine);
                } else if (isdigit(c)) {
                    string number = readNumber(i);
                    tokens.emplace_back(TokenType::NUMBER, number, curentLine);
                } else if (c == '"') {
                    string str = readString(i);
                    tokens.emplace_back(TokenType::STRING, str, curentLine);
                } else if (c == '#') {
                    skipComment(i, curentLine);
                } else if (c == ',') {
                    tokens.emplace_back(TokenType::COMMA, ",", curentLine);
                    i++;
                } else if (c == ';') {
                    tokens.emplace_back(TokenType::SEMICOLON, ";", curentLine);
                    i++;
                } else if (c == '=') {
                    if (input[i + 1] == '=') {
                        tokens.emplace_back(TokenType::EQUAL, "==", curentLine);
                        i += 2;
                    } else {
                        tokens.emplace_back(TokenType::ASSIGN, "=", curentLine);
                        i++;
                    }
                } else if (c == '<') {
                    if (input[i + 1] == '=') {
                        tokens.emplace_back(TokenType::LESS_E, "<=", curentLine);
                        i += 2;
                    } else {
                        tokens.emplace_back(TokenType::LESS, "<", curentLine);
                        i++;
                    }
                } else if (c == '>') {
                    if (input[i + 1] == '=') {
                        tokens.emplace_back(TokenType::GREATER_E, ">=", curentLine);
                        i += 2;
                    } else {
                        tokens.emplace_back(TokenType::GREATER, ">", curentLine);
                        i++;
                    }
                } else if (c == '!') {
                    if (input[i + 1] == '=') {
                        tokens.emplace_back(TokenType::NOT_EQUAL, "!=", curentLine);
                        i += 2;
                    } else {
                        tokens.emplace_back(TokenType::NOT, "!", curentLine);
                        i++;
                    }
                } else if (c == '+') {
                    tokens.emplace_back(TokenType::PLUS, "+", curentLine);
                    i++;
                } else if (c == '-') {
                    tokens.emplace_back(TokenType::MINUS, "-", curentLine);
                    i++;
                } else if (c == '*') {
                    tokens.emplace_back(TokenType::MUL, "*", curentLine);
                    i++;
                } else if (c == '/') {
                    tokens.emplace_back(TokenType::DIV, "/", curentLine);
                    i++;
                } else if (c == '(') {
                    tokens.emplace_back(TokenType::LPAREN, "(", curentLine);
                    i++;
                } else if (c == ')') {
                    tokens.emplace_back(TokenType::RPAREN, ")", curentLine);
                    i++;
                } else if (c == '[') {
                    tokens.emplace_back(TokenType::LBRACKET, "[", curentLine);
                    i++;
                } else if (c == ']') {
                    tokens.emplace_back(TokenType::RBRACKET, "]", curentLine);
                    i++;
                } else if (c == '{') {
                    tokens.emplace_back(TokenType::LBRACE, "{", curentLine);
                    i++;
                } else if (c == '}') {
                    tokens.emplace_back(TokenType::RBRACE, "}", curentLine);
                    i++;
                } else if (c == '.') {
                    tokens.emplace_back(TokenType::DOT_OP, ".", curentLine);
                    i++;
                } else {
                    tokens.emplace_back(TokenType::INVALID, string(1, c), curentLine);
                    i++;
                }
            }

            tokens.emplace_back(TokenType::EOF_, "", curentLine);
            return tokens;
        }

    private:
        string input;

        string readIdentifier(size_t& i) {
            size_t start = i;
            regex pattern("[a-zA-Z_]");
            
            while (i < input.length() && regex_match(string(1, input[i]), pattern)) {
                i++;
            }
            return input.substr(start, i - start);
        }

        string readNumber(size_t& i) {
            size_t start = i;
            bool isFloat = false;
            while (i < input.length() && (isdigit(input[i]) || input[i] == '.')) {
                if(input[i] == '.' && isFloat) break;
                else if(input[i] == '.') isFloat = true;
                i++;
            }
            return input.substr(start, i - start);
        }

        string readString(size_t& i) {
            size_t start = ++i;
            while (i < input.length()) {
                if (input[i] == '\\') {
                    i += 2; // skip the escape sequence
                } else if (input[i] == '"') {
                    i++;
                    break;
                } else {
                    i++;
                }
            }
            return input.substr(start, i - start - 1);
        }


        void skipComment(size_t& i, size_t& line){
            while (i < input.length()) {
                if (input[i] == '\n') {
                    line++;
                    i++;
                    break;
                }
                i++;
            }
        }

    TokenType getType(string identifier) {
        if (identifier == "var") {
            return TokenType::VAR;
        } else if (identifier == "const") {
            return TokenType::CONST;
        } else if (identifier == "null") {
            return TokenType::NULL_TYPE;
        } else if (identifier == "string") {
            return TokenType::STRING_TYPE;
        } else if (identifier == "number") {
            return TokenType::NUMBER_TYPE;
        } else if (identifier == "bool") {
            return TokenType::BOOL_TYPE;
        } else if (identifier == "true" || identifier == "false") {
            return TokenType::BOOL;
        } else if (identifier == "is") {
            return TokenType::IS;
        } else if (identifier == "readInt") {
            return TokenType::READ_INT;
        } else if (identifier == "readReal") {
            return TokenType::READ_REAL;
        } else if (identifier == "readString") {
            return TokenType::READ_STRING;
        } else if (identifier == "print") {
            return TokenType::PRINT;
        } else if (identifier == "if") {
            return TokenType::IF;
        } else if (identifier == "else") {
            return TokenType::ELSE;
        } else if (identifier == "then") {
            return TokenType::THEN;
        } else if (identifier == "end") {
            return TokenType::END;
        } else if (identifier == "print") {
            return TokenType::PRINT;
        } else if (identifier == "while") {
            return TokenType::WHILE_L;
        } else if (identifier == "for") {
            return TokenType::FOR_L;
        } else if (identifier == "loop") {
            return TokenType::LOOP;
        } else if (identifier == "func") {
            return TokenType::FUNC;
        } else if (identifier == "return") {
            return TokenType::RETURN;
        } else if (identifier == "do") {
            return TokenType::DO;
        } else if (identifier == "and") {
            return TokenType::AND;
        } else if (identifier == "or") {
            return TokenType::OR;
        } else if (identifier == "xor") {
            return TokenType::XOR;
        } else {
            return TokenType::IDENTIFIER;
        }
    }

};

int main() {
string input = R"(
# btw, this is comment ;)

# create mutable variable
var x = 1.23;

# reassign mutable variable to another type
x = "Hello"; # OK

# assign multiple variables in one line
var a = 30, b = false;

# can create variable via expression
var g = 10, h = 20;
var j = g * h / 2;

# initialize empty variable (it has special type `null`)
var c; # c is null

# create literal variable 
const y = "This is string";

# can check type of variable
var isYString = (y is string); # isYString = true

# can check for null
var isYNull = (y is null); # isYNull = false)";

Lexer lexer(input);
vector<Token> tokens = lexer.tokenize();

for (const Token& token : tokens) {
    cout << "(" << TokenTypeStr[token.getType()] << ", " << token.getLexeme() << ")" << endl;
}

return 0;
}
