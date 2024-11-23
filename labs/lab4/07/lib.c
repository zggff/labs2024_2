#include "lib.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long *cells_get(const Cells *c, const char *name) {
    int a = 0;
    int b = c->len - 1;
    while (a <= b) {
        // low + (high - low) / 2
        int m = a + (b - a) / 2;
        int r = strcmp(name, c->cells[m].name);
        if (r == 0)
            return &c->cells[m].val;
        else if (r < 0)
            b = m - 1;
        else
            a = m + 1;
    }
    return NULL;
}

int cell_cmp(const void *a_p, const void *b_p) {
    const MemoryCell *a = a_p;
    const MemoryCell *b = b_p;
    return strcmp(a->name, b->name);
}

int cells_set(Cells *c, const char *name, long val) {
    long *r = cells_get(c, name);
    if (r) {
        *r = val;
        return S_OK;
    }

    if (c->cap == 0) {
        size_t cap = CELLS_SIZE;
        c->cells = malloc(cap * sizeof(MemoryCell));
        if (!c->cells) {
            return S_MALLOC;
        }
        c->cap = cap;
    }

    if (c->len >= c->cap) {
        size_t cap = c->cap * 2;
        MemoryCell *cells = realloc(c->cells, cap * sizeof(MemoryCell));
        if (!cells) {
            return S_MALLOC;
        }
        c->cells = cells;
        c->cap = cap;
    }
    MemoryCell cell = {0};
    size_t len = strlen(name) + 1;
    cell.val = val;
    cell.name = malloc(len);
    if (!cell.name)
        return S_MALLOC;
    memcpy(cell.name, name, len);
    c->cells[c->len] = cell;
    c->len++;
    qsort(c->cells, c->len, sizeof(MemoryCell), cell_cmp);
    return S_OK;
}

int cells_free(Cells *c) {
    for (size_t i = 0; i < c->len; i++)
        free(c->cells[i].name);
    free(c->cells);
    return S_OK;
}
