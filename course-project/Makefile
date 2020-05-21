all: clean mkdir_build build exec

.PHONY: mkdir_build
mkdir_build:
	[ -d ./build ] | mkdir -p build

.PHONY: build
build:
	gcc ./src/app.c -o ./build/app -pthread -lX11

.PHONY: exec
exec:
	./build/app

.PHONY: clean
clean:
	rm -rf build
