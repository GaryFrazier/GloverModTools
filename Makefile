all:
	rm -rf bin
	mkdir -p bin
	gcc -Wall -mwin32 ${FLAGS} -o bin/gloverModTools src/*.c \

		

