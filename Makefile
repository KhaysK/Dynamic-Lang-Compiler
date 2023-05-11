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

clean:
	rm -rf build parser.tab.?pp
