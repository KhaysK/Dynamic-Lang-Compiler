all: build/Makefile
	cd build && cmake --build . -j $(nproc)

build/Makefile:
	mkdir -p build
	cd build && cmake -DDEBUG_MODE=ON ..

build_release:
	rm -rf build
	mkdir -p build
	cd build && cmake .. && cmake --build . -j $(nproc)

test: build_release | quick_test
	
quick_test:
	./scripts/test.sh build/compiler tests/

clean:
	rm -rf build parser.tab.?pp
