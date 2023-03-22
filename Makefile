CC=g++

all: bin/compiler

bin/compiler: obj/parser.tab.o obj/lexer.o obj/ast.o
	mkdir -p bin/
	$(CC) obj/parser.tab.o obj/lexer.o obj/ast.o -o bin/compiler

obj/lexer.o: lexer.cpp
	mkdir -p obj/
	$(CC) -c lexer.cpp -o obj/lexer.o

obj/parser.tab.o: parser.tab.cpp
	mkdir -p obj/
	$(CC) -c parser.tab.cpp -o obj/parser.tab.o

obj/ast.o: ast.cpp
	mkdir -p obj/
	$(CC) -c ast.cpp -o obj/ast.o

parser.tab.cpp: parser.ypp
	bison -d parser.ypp -Wcounterexamples

clean:
	rm -rf obj/ bin/ parser.tab.?pp
