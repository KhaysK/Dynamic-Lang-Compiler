#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum TokenType : int {
    IDENTIFIER = 0,
    NUMBER,
    STRING,
    BOOL,
    VAR,
    CONST,
    IS,
    NULL_,
    COMMA,
    SEMICOLON,
    ASSIGN,
    PLUS,
    MINUS,
    MUL,
    DIV,
    LPAREN,
    RPAREN,
    EOF_
};

const string TokenTypeStr[] = {
    [TokenType::IDENTIFIER] = "IDENTIFIER",
    [TokenType::NUMBER] = "NUMBER",
    [TokenType::STRING] = "STRING",
    [TokenType::BOOL] = "BOOL",
    [TokenType::VAR] = "VAR",
    [TokenType::CONST] = "CONST",
    [TokenType::IS] = "IS",
    [TokenType::NULL_] = "NULL_",
    [TokenType::COMMA] = "COMMA",
    [TokenType::SEMICOLON] = "SEMICOLON",
    [TokenType::ASSIGN] = "ASSIGN",
    [TokenType::PLUS] = "PLUS",
    [TokenType::MINUS] = "MINUS",
    [TokenType::MUL] = "MUL",
    [TokenType::DIV] = "DIV",
    [TokenType::LPAREN] = "LPAREN",
    [TokenType::RPAREN] = "RPAREN",
    [TokenType::EOF_] = "EOF_",
};

class Token {
public:
    Token(TokenType type, string lexeme, int line) : type(type), lexeme(lexeme), line(line) {}

    TokenType getType() const { return type; }

    string getLexeme() const { return lexeme; }

private:
    TokenType type;
    string lexeme;
    int line;
};

class Lexer {
public:
    Lexer(string input) : input(input) {}

    vector<Token> tokenize() {
        vector<Token> tokens;
        size_t i = 0;
        size_t curentLine = 0;

        while (i < input.length()) {
            char c = input[i];

            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
                if(c == '\n') curentLine++;
                i++;
            } else if (isalpha(c)) {
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
                skipComment(i);
            } else if (c == ',') {
                tokens.emplace_back(TokenType::COMMA, ",", curentLine);
                i++;
            } else if (c == ';') {
                tokens.emplace_back(TokenType::SEMICOLON, ";", curentLine);
                i++;
            } else if (c == '=') {
                if (input[i + 1] == '=') {
                    tokens.emplace_back(TokenType::IS, "==", curentLine);
                    i += 2;
                } else {
                    tokens.emplace_back(TokenType::ASSIGN, "=", curentLine);
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
            } else {
                cerr << "Invalid character: " << c << endl;
                exit(1);
            }
        }

        tokens.emplace_back(TokenType::EOF_, "", curentLine);
        return tokens;
    }

private:
    string input;

    string readIdentifier(size_t& i) {
        size_t start = i;
        while (i < input.length() && (isalnum(input[i]) || input[i] == '_')) {
            i++;
        }
        return input.substr(start, i - start);
    }

    string readNumber(size_t& i) {
        size_t start = i;
        while (i < input.length() && (isdigit(input[i]) || input[i] == '.')) {
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


    void skipComment(size_t& i){
        while (i < input.length()) {
            if (input[i] == '\n') {
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
        return TokenType::NULL_;
    } else if (identifier == "true" || identifier == "false") {
        return TokenType::BOOL;
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
