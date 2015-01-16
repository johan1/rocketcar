all: app

run: app
	./build/LevelEditor

clean:
	rm -rf ./build

app:
	mkdir -p build && cd build && qmake -qt4 -makefile .. && make

.PHONY: all run clean app
