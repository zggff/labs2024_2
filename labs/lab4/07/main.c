#include <stdio.h>
#include <string.h>
#include "tokenize.h"
#include "lib.h"

int print_error(const char *s) {
    fprintf(stderr, "ERROR: variable [%s] is not initialized\n", s);
    fflush(stdout);
    return S_INVALID_INPUT;
}

int handle_print(char **toks, int n, Cells *cells) {
    if (n > 2) {
        long *val = cells_get(cells, toks[1]);
        if (val) {
            printf("[%s] = %ld\n\n", toks[1], *val);
            fflush(stdout);
            return S_OK;
        } else {
            return print_error(toks[1]);
        }
    }

    for (size_t i = 0; i < cells->len; i++) {
        MemoryCell c = cells->cells[i];
        printf("[%s] = %ld\n", c.name, c.val);
    }
    printf("\n");
    fflush(stdout);
    return S_OK;
}

int handle_math(char **toks, int n, Cells *cells) {
    if (n <= 4) {
        if (isnumber(*toks[2])) {
            long val;
            sscanf(toks[2], "%ld", &val);
            return cells_set(cells, toks[0], val);
        }
        long *val = cells_get(cells, toks[2]);
        if (!val)
            return print_error(toks[2]);
        return cells_set(cells, toks[0], *val);
    }
    long a = 0;
    long b = 0;
    if (isnumber(*toks[2])) {
        sscanf(toks[2], "%ld", &a);
    } else {
        long *val = cells_get(cells, toks[2]);
        if (!val)
            return print_error(toks[2]);
        a = *val;
    }

    if (isnumber(*toks[4])) {
        sscanf(toks[4], "%ld", &b);
    } else {
        long *val = cells_get(cells, toks[4]);
        if (!val)
            return print_error(toks[4]);
        b = *val;
    }

    switch (*toks[3]) {
    case '+':
        return cells_set(cells, toks[0], a + b);
    case '-':
        return cells_set(cells, toks[0], a - b);
    case '*':
        return cells_set(cells, toks[0], a * b);
    case '/':
        if (b == 0) {
            fprintf(stderr, "ERROR: can not divide by zero\n");
            fflush(stderr);
            return S_INVALID_INPUT;
        }
        return cells_set(cells, toks[0], a / b);
    case '%':
        if (b == 0) {
            fprintf(stderr, "ERROR: can not divide by zero\n");
            fflush(stderr);
            return S_INVALID_INPUT;
        }
        return cells_set(cells, toks[0], a % b);

    default:
        return S_INVALID_INPUT;
    }
}

int main(int argc, char *argw[]) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: input file not provided");
        fflush(stderr);
        return 1;
    }
    FILE *f = fopen(argw[1], "r");
    if (!f) {
        fprintf(stderr, "ERROR: failed to open file: [%s]\n", argw[1]);
        fflush(stderr);
        return 1;
    }
    char **toks = NULL;
    size_t tok_cap = 0;
    char buf[BUF_SIZE] = {0};
    int off = 0;

    int res = 0;

    Cells cells = {0};

    while (!res) {
        int n = token_parse_file(&toks, &tok_cap, buf, &off, f);
        if (n <= 0)
            break;
        if (strcmp(toks[0], "print") == 0) {
            res = handle_print(toks, n, &cells);
        } else {
            res = handle_math(toks, n, &cells);
        }

        for (int i = 0; i < n; i++) {
            free(toks[i]);
            toks[i] = NULL;
        }
    }
    free(toks);
    cells_free(&cells);

    fclose(f);
    return res;
}
