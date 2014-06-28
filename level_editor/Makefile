all: app

run: app
	./build/LevelEditor

clean:
	rm -rf ./build

app:
	mkdir -p build && cd build && qmake -makefile .. && make

.PHONY: all run clean app
