CC=clang++
CFLAGS=-Wno-c99-designator

all: bin/compiler

bin/compiler: obj/parser.tab.o obj/lexer.o obj/ast.o obj/MemoryKernel.o
	mkdir -p bin/
	$(CC) $(CFLAGS) obj/parser.tab.o obj/lexer.o obj/ast.o obj/MemoryKernel.o -o bin/compiler

obj/lexer.o: lexer.cpp
	mkdir -p obj/
	$(CC) $(CFLAGS) -c lexer.cpp -o obj/lexer.o

obj/parser.tab.o: parser.tab.cpp
	mkdir -p obj/
	$(CC) $(CFLAGS) -c parser.tab.cpp -o obj/parser.tab.o

obj/ast.o: ast.cpp
	mkdir -p obj/
	$(CC) $(CFLAGS) -c ast.cpp -o obj/ast.o

obj/MemoryKernel.o: MemoryKernel.cpp MemoryKernel.hpp ast.hpp
	mkdir -p obj/
	$(CC) $(CFLAGS) -c MemoryKernel.cpp -o obj/MemoryKernel.o

parser.tab.cpp: parser.ypp
	bison -d parser.ypp -Wcounterexamples

clean:
	rm -rf obj/ bin/ parser.tab.?pp
