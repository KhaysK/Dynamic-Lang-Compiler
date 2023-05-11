all: build/Makefile
	make -C build/

build/Makefile:
	mkdir -p build
	cd build && cmake -DDEBUG_MODE=ON ..

build_release:
	rm -rf build
	mkdir -p build
	cd build && cmake ..
	make -C build/

test: build_release | quick_test
	
quick_test:
	./scripts/test.sh build/compiler tests/

clean:
	rm -rf build parser.tab.?pp
