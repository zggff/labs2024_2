CC = clang
CFLAGS = -std=gnu17 -pedantic -Wall -Wextra -g
TARGET = main.out


SOURCES=$(wildcard *.c)
HEADERS=$(wildcard *.h)

.PHONY: clean run build test

build: ${TARGET}

${TARGET}: ${SOURCES} ${HEADERS} 
	${CC} ${CFLAGS} -o $@ ${SOURCES} 

test: build
	@python3 ./test.py

clean:
	@rm -f ./*.o
	@rm -f ${TARGET}
	@rm -rf ${TARGET}.dSYM
