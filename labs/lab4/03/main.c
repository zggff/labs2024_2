#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "tokenize.h"
#include "trie.h"

#define FREE_ALL()                                                             \
    {                                                                          \
        for (int i = 0; toks[i]; i++) {                                        \
            free(toks[i]);                                                     \
            toks[i] = NULL;                                                    \
        }                                                                      \
        free(toks);                                                            \
        fclose(f);                                                             \
    }

typedef int (*handle)(Polynom *p, char **toks, int tok_len);

int handle_two(Polynom *p, char **toks, int tok_len) {
    const char *command = toks[0];
    Polynom a = {0};
    Polynom b = {0};
    tok_len -= 2;
    toks += 2;
    int off = 0;
    int r = polynom_parse_tokens(&a, toks, tok_len, &off);
    if (r)
        return r;
    toks += off;
    tok_len -= off;
    off = 0;
    if (strcmp(toks[0], ",") == 0) {
        polynom_free(p);
        int r = polynom_parse_tokens(&b, toks + 1, tok_len - 1, &off);
        if (r) {
            polynom_free(&a);
            return r;
        }
        toks += off + 1;
        tok_len -= off + 1;
    } else if (strcmp(toks[0], ")") == 0) {
        b = a;
        a = *p;
        toks++;
        tok_len--;
    } else {
        token_print_error(stderr, toks[0], ", | )");
        return S_INVALID_INPUT;
    }

    memset(p, 0, sizeof(Polynom));
    int res = 0;
    if (strcmp(command, "Add") == 0) {
        res = polynom_add(p, &a, &b);
    } else if (strcmp(command, "Sub") == 0) {
        res = polynom_sub(p, &a, &b);
    } else {
        res = polynom_mul(p, &a, &b);
    }
    if (!res)
        polynom_print(p);
    polynom_free(&a);
    polynom_free(&b);
    return res;
}

int check_val(const char *s) {
    while (*s) {
        if (!isalnum(*s))
            return 0;
        s++;
    }
    return 1;
}

int handle_eval_callback(const char *k, long v, void *ptr) {
    (void)ptr;
    (void)k;
    free((double *)v);
    return S_OK;
}

int handle_eval(Polynom *p, char **toks, int tok_len) {
    tok_len -= 2;
    toks += 2;
    Polynom a = {0};
    if (strcmp(toks[0], "{") != 0) {
        int off = 0;
        int r = polynom_parse_tokens(&a, toks, tok_len, &off);
        if (r)
            return r;
        tok_len -= off;
        toks += off;
        if (strcmp(toks[0], ",") != 0) {
            token_print_error(stderr, toks[0], ",");
            return S_INVALID_INPUT;
        }
        tok_len--;
        toks++;
    } else {
        a = *p;
    }
    if (strcmp(toks[0], "{") != 0) {
        token_print_error(stderr, toks[0], "{");
        polynom_free(&a);
        return S_INVALID_INPUT;
    }
    toks++;
    tok_len--;
    int i = 0;
    Trie vals = {0};
    while (i < tok_len - 2 && strcmp(toks[i], "}") != 0) {
        if (!check_val(toks[i])) {
            fprintf(stderr, "ERROR: invalid variable [%s]\n", toks[i]);
            fflush(stderr);
            trie_for_each(&vals, handle_eval_callback, NULL);
            trie_free(&vals);
            polynom_free(&a);
            return S_INVALID_INPUT;
        }
        char *var = toks[i];
        if (strcmp(toks[i + 1], ":") != 0) {
            trie_for_each(&vals, handle_eval_callback, NULL);
            token_print_error(stderr, toks[0], ":");
            polynom_free(&a);
            return S_INVALID_INPUT;
        }
        bool neg = strcmp(toks[i + 2], "-") == 0;
        if (neg)
            i++;
        char *tmp;
        double f = strtod(toks[i + 2], &tmp);
        if (*tmp != 0) {
            fprintf(stderr, "ERROR: failed to parse [%s] as number\n",
                    toks[i + 2]);
            fflush(stderr);
            trie_for_each(&vals, handle_eval_callback, NULL);
            trie_free(&vals);
            polynom_free(&a);
            return S_INVALID_INPUT;
        }
        f = neg ? -f : f;
        double *f_ptr = malloc(sizeof(double));
        if (!f_ptr)
            return S_MALLOC;
        *f_ptr = f;
        trie_set(&vals, var, (long)f_ptr);
        i += 3;
        if (strcmp(toks[i], ",") == 0)
            i++;
    }
    toks += i;
    tok_len -= i;
    if (tok_len < 1 || strcmp(toks[0], "}") != 0) {
        token_print_error(stderr, toks[0], "}");
        trie_for_each(&vals, handle_eval_callback, NULL);
        trie_free(&vals);
        polynom_free(&a);
        return S_INVALID_INPUT;
    }
    if (tok_len < 2 || strcmp(toks[1], ")") != 0) {
        token_print_error(stderr, toks[1], ")");
        trie_for_each(&vals, handle_eval_callback, NULL);
        trie_free(&vals);
        polynom_free(&a);
        return S_INVALID_INPUT;
    }

    double res = 0;
    int r = polynom_eval(&res, &a, &vals);
    if (!r)
        printf("%f\n", res);

    trie_for_each(&vals, handle_eval_callback, NULL);
    trie_free(&vals);
    polynom_free(&a);
    memset(p, 0, sizeof(Polynom));
    return r;
}

int handle_one_name(Polynom *p, char **toks, int tok_len) {
    const char *command = toks[0];
    Polynom a = {0};
    tok_len -= 2;
    toks += 2;
    bool two_params = false;
    for (int i = 0; !two_params && i < tok_len; i++)
        two_params |= strcmp(toks[i], ",") == 0;
    if (two_params) {
        int off = 0;
        int r = polynom_parse_tokens(&a, toks, tok_len, &off);
        if (r)
            return r;
        tok_len -= off;
        toks += off;
        if (strcmp(toks[0], ",") != 0) {
            token_print_error(stderr, toks[0], ",");
            return S_INVALID_INPUT;
        }
        tok_len--;
        toks++;
    } else {
        a = *p;
    }
    if (tok_len < 2 || strcmp(toks[1], ")") != 0) {
        token_print_error(stderr, toks[1], ")");
        polynom_free(&a);
        return S_INVALID_INPUT;
    }

    const char *var = toks[0];
    int res = 0;
    memset(p, 0, sizeof(Polynom));
    if (strcmp(command, "Deriv") == 0) {
        res = polynom_deriv(p, &a, var);
    } else if (strcmp(command, "Prim") == 0) {
        res = polynom_prim(p, &a, var);
    } else {
        res = polynom_grad(p, &a, var);
    }
    if (!res)
        polynom_print(p);
    polynom_free(&a);
    return S_OK;
}

int print_callback(const char *k, long v, void *ptr) {
    (void)ptr;
    printf("[%s] = %ld, ", k, v);
    return S_OK;
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

    char *ops[] = {"Add", "Sub", "Mult", "Eval", "Deriv", "Grad", "Prim"};
    handle handles[] = {handle_two,     handle_two,      handle_two,
                        handle_eval,    handle_one_name, handle_one_name,
                        handle_one_name};

    char **toks = NULL;
    size_t tok_cap = 0;
    char buf[BUF_SIZE] = {0};
    int off = 0;

    Polynom p = {0};

    int res = 0;

    while (!res) {
        int n = token_parse_file(&toks, &tok_cap, buf, &off, f);
        if (n <= 0)
            break;
        handle *h = NULL;
        for (size_t i = 0; !h && i < sizeof(ops) / sizeof(char *); i++) {
            if (strcmp(toks[0], ops[i]) == 0)
                h = &handles[i];
        }
        if (n <= 1 || strcmp(toks[1], "(") != 0) {
            token_print_error(stderr, toks[1], "(");
            res = S_INVALID_INPUT;
            break;
        }
        if (n <= 3 || strcmp(toks[n - 1], ";") != 0) {
            token_print_error(stderr, toks[n - 1], ";");
            res = S_INVALID_INPUT;
            break;
        }

        if (h) {
            res = (*h)(&p, toks, n);
            fflush(stdout);
        } else {
            fprintf(stderr, "ERROR: unsupported operation: {%s}\n", toks[0]);
            fprintf(stderr, "\tsupported: {");
            for (size_t i = 0; i < sizeof(ops) / sizeof(char *); i++) {
                if (i > 0)
                    fprintf(stderr, ", ");
                fprintf(stderr, "%s", ops[i]);
            }
            fprintf(stderr, "}\n");
            fflush(stderr);
            res = S_INVALID_INPUT;
        }

        for (int i = 0; i < n; i++) {
            free(toks[i]);
            toks[i] = NULL;
        }
    }
    polynom_free(&p);
    free(toks);
    fclose(f);
    return res;
}
