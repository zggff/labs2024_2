#pragma once
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "trie.h"

typedef struct Monom {
    double coef;
    Trie vars;
} Monom;

typedef struct Polynom Polynom;
struct Polynom {
    Polynom *prev;
    Monom cur;
    Polynom *next;
};

int polynom_free(Polynom *p);
int polynom_parse_str(Polynom *p, const char *s);
int polynom_parse_tokens(Polynom *p, char **toks, int toks_len, int *off);
int polynom_print(const Polynom *p);

int polynom_add(Polynom *p, const Polynom *a, const Polynom *b);
int polynom_sub(Polynom *p, const Polynom *a, const Polynom *b);
int polynom_mul(Polynom *p, const Polynom *a, const Polynom *b);

int polynom_eval(double *res, const Polynom *a, const Trie *vals);

int polynom_deriv(Polynom *p, const Polynom *a, const char *var);
int polynom_prim(Polynom *p, const Polynom *a, const char *var);
int polynom_grad(Polynom *p, const Polynom *a, const char *var);
