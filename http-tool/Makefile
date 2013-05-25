

FLAGS=-g -D LINUX

main: clean
	mkdir bin
	gcc $(FLAGS) -lpthread *.c -o bin/http-tool


clean:
	mkdir -p bin
	rm -r bin
