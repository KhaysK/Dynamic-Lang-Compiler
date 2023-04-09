#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

#include "parser.tab.hpp"
#include "ast.hpp"

enum TokenType : int {
    EOF_ = 0,
    IDENTIFIER,
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
    [TokenType::EOF_] = "EOF_",
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

vector<Token> tokens;
int idx = 0;

yy::parser::symbol_type get_next_token() {
    const Token& token = tokens[idx];
    TokenType type = token.getType();
    // cout << TokenTypeStr[type] << '\n';
    idx++;
    switch (type) {
        case TokenType::IDENTIFIER:
            return yy::parser::make_IDENTIFIER(token.getLexeme());
            break;
    
        case TokenType::NUMBER:
            return yy::parser::make_NUMBER(token.getLexeme());
            break;
        
        case TokenType::NUMBER_TYPE:
            return yy::parser::make_NUMBER_TYPE();
            break;
        
        case TokenType::STRING:
            return yy::parser::make_STRING(token.getLexeme());
            break;

        case TokenType::STRING_TYPE:
            return yy::parser::make_STRING_TYPE();
            break;

        case TokenType::BOOL:
            return yy::parser::make_BOOL(token.getLexeme());
            break;

        case TokenType::BOOL_TYPE:
            return yy::parser::make_BOOL_TYPE();
            break;

        case TokenType::VAR:
            return yy::parser::make_VAR();
            break;

        case TokenType::CONST:
            return yy::parser::make_CONST();
            break;

        case TokenType::IS:
            return yy::parser::make_IS();
            break;

        case TokenType::NULL_TYPE:
            return yy::parser::make_NULL_TYPE();
            break;

        case TokenType::COMMA:
            return yy::parser::make_COMMA();
            break;

        case TokenType::SEMICOLON:
            return yy::parser::make_SEMICOLON();
            break;

        case TokenType::ASSIGN:
            return yy::parser::make_ASSIGN();
            break;

        case TokenType::PLUS:
            return yy::parser::make_PLUS();
            break;

        case TokenType::MINUS:
            return yy::parser::make_MINUS();
            break;

        case TokenType::MUL:
            return yy::parser::make_MUL();
            break;

        case TokenType::DIV:
            return yy::parser::make_DIV();
            break;

        case TokenType::LPAREN:
            return yy::parser::make_LPAREN();
            break;

        case TokenType::RPAREN:
            return yy::parser::make_RPAREN();
            break;

        case TokenType::LBRACKET:
            return yy::parser::make_LBRACKET();
            break;

        case TokenType::RBRACKET:
            return yy::parser::make_RBRACKET();
            break;

        case TokenType::LBRACE:
            return yy::parser::make_LBRACE();
            break;

        case TokenType::RBRACE:
            return yy::parser::make_RBRACE();
            break;

        case TokenType::READ_INT:
            return yy::parser::make_READ_INT();
            break;

        case TokenType::READ_REAL:
            return yy::parser::make_READ_REAL();
            break;

        case TokenType::READ_STRING:
            return yy::parser::make_READ_STRING();
            break;

        case TokenType::PRINT:
            return yy::parser::make_PRINT();
            break;

        case TokenType::IF:
            return yy::parser::make_IF();
            break;

        case TokenType::ELSE:
            return yy::parser::make_ELSE();
            break;

        case TokenType::THEN:
            return yy::parser::make_THEN();
            break;

        case TokenType::END:
            return yy::parser::make_END();
            break;

        case TokenType::LESS:
            return yy::parser::make_LESS();
            break;

        case TokenType::LESS_E:
            return yy::parser::make_LESS_E();
            break;

        case TokenType::GREATER:
            return yy::parser::make_GREATER();
            break;

        case TokenType::GREATER_E:
            return yy::parser::make_GREATER_E();
            break;

        case TokenType::EQUAL:
            return yy::parser::make_EQUAL();
            break;

        case TokenType::NOT_EQUAL:
            return yy::parser::make_NOT_EQUAL();
            break;

        case TokenType::NOT:
            return yy::parser::make_NOT();
            break;

        case TokenType::WHILE_L:
            return yy::parser::make_WHILE_L();
            break;

        case TokenType::FOR_L:
            return yy::parser::make_FOR_L();
            break;

        case TokenType::LOOP:
            return yy::parser::make_LOOP();
            break;

        case TokenType::FUNC:
            return yy::parser::make_FUNC();
            break;

        case TokenType::RETURN:
            return yy::parser::make_RETURN();
            break;

        case TokenType::DO:
            return yy::parser::make_DO();
            break;

        case TokenType::DOT_OP:
            return yy::parser::make_DOT_OP();
            break;

        case TokenType::INVALID:
            cout << "lexer error on token " << '"' << token.getLexeme() << '"' << " (line: " << token.getLine() << ")\n";
            exit(1);
            return yy::parser::make_INVALID();
            break;

        case TokenType::AND:
            return yy::parser::make_AND();
            break;

        case TokenType::OR:
            return yy::parser::make_OR();
            break;

        case TokenType::XOR:
            return yy::parser::make_XOR();
            break;

        case TokenType::EOF_:
            return yy::parser::make_EOF_();
            break;

        default:
            return yy::parser::make_INVALID();
            break;
    }
}

int main() {
    string filename = "tests/7.nnl";

    ifstream file(filename);
    string line;
    string input;

    while (getline(file, line)) {
        input += line + '\n';
    }

    cout << input << '\n';

    file.close();

    Lexer lexer(input);
    tokens = lexer.tokenize();
    AST::ASTNode* ast_root;

    yy::parser p(&ast_root);
    p.parse();

    cout << ast_root->str() << '\n';

    return 0;
}

namespace yy
{
    parser::symbol_type yylex()
    {
        return get_next_token();
    }
}
