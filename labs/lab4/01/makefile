CC = clang
CFLAGS = -std=gnu17 -pedantic -Wall -Wextra -g
TARGET = main.out

.PHONY: clean run build test

build: ${TARGET}

${TARGET}: main.c
	${CC} ${CFLAGS} -o $@ $^

test:
	@python3 ./test.py

clean:
	@rm -f ./*.o
	@rm -f ${TARGET}
	@rm -rf ${TARGET}.dSYM
