CC=clang++
CFLAGS=-Wno-c99-designator -ggdb

all: bin/compiler

bin/compiler: obj/parser.tab.o obj/lexer.o obj/ast.o obj/ASTContext.o
	mkdir -p bin/
	$(CC) $(CFLAGS) obj/parser.tab.o obj/lexer.o obj/ast.o obj/ASTContext.o -o bin/compiler

obj/lexer.o: lexer.cpp
	mkdir -p obj/
	$(CC) $(CFLAGS) -c lexer.cpp -o obj/lexer.o

obj/parser.tab.o: parser.tab.cpp
	mkdir -p obj/
	$(CC) $(CFLAGS) -c parser.tab.cpp -o obj/parser.tab.o

obj/ast.o: ast.cpp
	mkdir -p obj/
	$(CC) $(CFLAGS) -c ast.cpp -o obj/ast.o

obj/ASTContext.o: ASTContext.hpp ast.hpp ASTContext.cpp
	mkdir -p obj/
	$(CC) $(CFLAGS) -c ASTContext.cpp -o obj/ASTContext.o

parser.tab.cpp: parser.ypp
	bison -d parser.ypp -Wcounterexamples

clean:
	rm -rf obj/ bin/ parser.tab.?pp
