#pragma once
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef enum Status {
    S_OK = 0,
    S_INVALID_INPUT = 1,
    S_MALLOC = 2,
} Status;

typedef struct MemoryCell {
    char *name;
    long val;
} MemoryCell;

int validate_name(const char* s);


#define CELLS_SIZE 128

typedef struct Cells {
    size_t len;
    size_t cap;
    MemoryCell *cells;
} Cells;

int cells_set(Cells* c, const char *name, long val);
long* cells_get(const Cells* c, const char *name);
int cells_free(Cells* c);
