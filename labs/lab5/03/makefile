CC = clang++
CFLAGS = -std=gnu++23 -pedantic -Wall -Wextra -g
TARGET = main.out


SOURCES=$(wildcard *.cpp)
HEADERS=$(wildcard *.hpp)

.PHONY: clean run build test

build: ${TARGET}

${TARGET}: ${SOURCES} ${HEADERS} 
	${CC} ${CFLAGS} -o $@ ${SOURCES} 

test: build
	@python3 ./test.py

clean:
	@rm -f ${TARGET}
	@rm -rf ${TARGET}.dSYM


