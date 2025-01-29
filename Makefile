all: clean build run

clean:
	rm -f sammy

build:
	clang sammy.c -o sammy

debug:
	clang sammy.c -o sammy_debug -g
	lldb ./sammy_debug

run:
	clear
	./sammy