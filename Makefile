CC = g++
CFLAGS= -Wall 
INCLUDE = -Ilib/GLAD/include -I/usr/include/SDL2
LIBS = -lSDL2 -ldl

lsystem:
	@mkdir -p build
	$(CC) $(CFLAGS) lib/GLAD/src/glad.c lsystem.cpp $(INCLUDE) -o build/lsystem $(LIBS)

.PHONY: clean run
clean:
	@rm -rf build
run:
	make && ./build/lsystem
