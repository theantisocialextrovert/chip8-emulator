INCLUDES= -I ./include
FLAGS= -g

OBJECTS=./build/chip8.o
all: ${OBJECTS}
	gcc ${FLAGS} ${INCLUDES} ./src/main.c ${OBJECTS} -L ./lib -lmingw32 -lSDL2main -lSDL2 -o ./bin/main

./build/chip8.o: ./src/chip8.c
	gcc ${FLAGS} ${INCLUDES} ./src/chip8.c -c -o ./build/chip8.o

clean:
	del build\*