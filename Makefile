all: parser.ypp
	bison -d parser.ypp
	g++ real_lexer.cpp parser.tab.cpp -o compiler