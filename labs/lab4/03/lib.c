#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenize.h"
#include "trie.h"
#include "lib.h"

int monom_print_callback(const char *k, long val, void *ptr) {
    FILE *f = ptr;
    fprintf(f, " * %s^%ld", k, val);
    return S_OK;
}

int monom_print(FILE *f, const Monom *m) {
    fprintf(f, "%f", m->coef);
    int res = trie_for_each(&m->vars, monom_print_callback, f);
    return res;
}

int monom_parse_tokens(Monom *p, char **toks, int toks_len, int *off) {
    p->coef = 1;
    bool neg = false;
    if (*off < toks_len && strcmp(toks[*off], "-") == 0) {
        p->coef *= -1;
        (*off)++;
    }
    int i;
    for (i = *off; i < toks_len; i++) {
        if (strcmp(toks[i], "-") == 0 && i + 1 < toks_len &&
            isnumber(toks[i + 1][0])) {
            i++;
            neg = true;
        }
        if (isnumber(toks[i][0])) {
            char *end;
            double c = strtof(toks[i], &end);
            if (*end != 0) {
                fprintf(stderr, "ERROR: failed to parse [%s] as float\n",
                        toks[i]);
                fflush(stderr);
                return S_INVALID_INPUT;
            }
            if (neg)
                c *= -1;
            p->coef *= c;

        } else if (isalpha(toks[i][0])) {
            int power = 1;
            char *name = toks[i];
            if (i + 2 < toks_len && strcmp(toks[i + 1], "^") == 0) {
                if (i + 3 < toks_len && strcmp(toks[i + 2], "-") == 0) {
                    neg = true;
                    i++;
                }
                char *s_power = toks[i + 2];
                char *end;
                power = strtol(s_power, &end, 10);
                if (*end != 0) {
                    fprintf(stderr, "ERROR: failed to parse [%s] as int\n",
                            s_power);
                    fflush(stderr);
                    return S_INVALID_INPUT;
                }
                if (neg)
                    power *= -1;
                i += 2;
            }
            long current = trie_get(&p->vars, name);
            trie_set(&p->vars, name, current + power);
        } else {
            fprintf(stderr, "ERROR: unexpected token [%s]\n", toks[i]);
            fflush(stderr);
            return S_INVALID_INPUT;
        }
        i++;
        if (i < toks_len) {
            if (strcmp(toks[i], "*") != 0) {
                break;
            }
        }
        neg = false;
    }
    *off = i;
    return S_OK;
}

int polynom_add_monom(Polynom *p, Monom m) {
    // monom_print(&m);
    // printf("\n");
    Polynom *tmp = p;
    Polynom *prev = NULL;
    while (tmp) {
        if (tmp->cur.coef == 0 || trie_eq(&tmp->cur.vars, &m.vars))
            break;
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp) {
        if (tmp->cur.coef == 0)
            tmp->cur = m;
        else
            tmp->cur.coef += m.coef;

        return S_OK;
    }
    prev->next = malloc(sizeof(Polynom));
    if (prev->next == NULL)
        return S_MALLOC;
    prev->next->cur = m;
    prev->next->next = NULL;
    prev->next->prev = prev;
    return S_OK;
}

int _monom_validate_callback(const char *k, long v, void *ptr) {
    bool *res = ptr;
    if (v < 0)
        *res = true;
    (void)k;
    return 0;
}

int polynom_parse_tokens(Polynom *p, char **toks, int toks_len, int *off) {
    bool end = false;
    while (!end) {
        Monom m = {0};
        int r = monom_parse_tokens(&m, toks, toks_len, off);
        if (r)
            return r;
        bool neg = false;
        trie_for_each(&m.vars, _monom_validate_callback, &neg);
        if (neg) {
            fprintf(stderr, "ERROR: monom has powers < 0: {");
            monom_print(stderr, &m);
            fprintf(stderr, "}\n");
            fflush(stderr);
            polynom_free(p);
            trie_free(&m.vars);
            return S_INVALID_INPUT;
        }
        bool valid = false;
        if (*off >= toks_len || strcmp(toks[*off], ",") == 0 ||
            strcmp(toks[*off], ")") == 0) {
            valid = true;
            end = true;
        }
        if (!valid &&
            (strcmp(toks[*off], "+") == 0 || strcmp(toks[*off], "-") == 0)) {
            valid = true;
        }
        if (*off < toks_len && strcmp(toks[*off], "+") == 0) {
            (*off)++;
        }
        if (!valid) {
            fprintf(stderr, "ERROR: unexpected token [%s]\n", toks[*off]);
            fflush(stderr);
            polynom_free(p);
            trie_free(&m.vars);
            return S_INVALID_INPUT;
        }
        r = polynom_add_monom(p, m);
        if (r)
            return r;
    }
    return S_OK;
}

int polynom_parse_str(Polynom *p, const char *s) {
    size_t tok_len = 0;
    char **toks = NULL;
    int n = token_parse_str(&toks, &tok_len, s);
    // int off = 0;
    // int n = token_parse_list(&toks, &tok_len, s, &off, token_str_getter,
    // NULL);
    int res_off = 0;
    int res = polynom_parse_tokens(p, toks, n, &res_off);
    for (int i = 0; i < n; i++) {
        free(toks[i]);
    }
    free(toks);
    return res;
}

int monom_parse_str(Monom *p, const char *s) {
    size_t tok_len = 0;
    char **toks = NULL;
    int n = token_parse_str(&toks, &tok_len, s);
    int res_off = 0;
    int res = monom_parse_tokens(p, toks, n, &res_off);
    for (int i = 0; i < n; i++) {
        free(toks[i]);
    }
    free(toks);
    return res;
}

int polynom_print(const Polynom *p) {
    int i = 0;
    while (p) {
        if (p->cur.coef == 0) {
            p = p->next;
            continue;
        }
        if (i > 0)
            printf(" + ");
        monom_print(stdout, &p->cur);
        i = 1;
        p = p->next;
    };
    if (i == 0)
        printf("%f", 0.0);
    printf("\n");
    return S_OK;
}

int polynom_free(Polynom *p) {
    trie_free(&p->cur.vars);
    p = p->next;
    while (p) {
        Polynom *next = p->next;
        trie_free(&p->cur.vars);
        free(p);
        p = next;
    }
    return 0;
}

int _polynom_add_mult(Polynom *p, const Polynom *a, const Polynom *b,
                      float mult) {
    while (a) {
        Monom m = {0};
        m.coef = a->cur.coef;
        trie_dup(&m.vars, &a->cur.vars);
        int r = polynom_add_monom(p, m);
        if (r)
            return r;
        a = a->next;
    }
    while (b) {
        Monom m = {0};
        m.coef = mult * b->cur.coef;
        trie_dup(&m.vars, &b->cur.vars);
        int r = polynom_add_monom(p, m);
        if (r)
            return r;
        b = b->next;
    }
    return S_OK;
}

int polynom_add(Polynom *p, const Polynom *a, const Polynom *b) {
    return _polynom_add_mult(p, a, b, 1);
}

int polynom_sub(Polynom *p, const Polynom *a, const Polynom *b) {
    return _polynom_add_mult(p, a, b, -1);
}

int _monom_mult_callback(const char *k, long v, void *ptr) {
    Trie *m = ptr;
    long cur = trie_get(m, k);
    return trie_set(m, k, cur + v);
}

int monom_mult(Monom *m, const Monom *a, const Monom *b) {
    m->coef = a->coef * b->coef;
    int r = trie_dup(&m->vars, &a->vars);
    if (r)
        return r;
    r = trie_for_each(&b->vars, _monom_mult_callback, &m->vars);
    return r;
}

int polynom_mul(Polynom *p, const Polynom *a, const Polynom *b) {
    while (a) {
        const Polynom *c = b;
        while (c) {
            Monom m = {0};
            int r = monom_mult(&m, &a->cur, &c->cur);
            if (r)
                return r;
            r = polynom_add_monom(p, m);
            if (r)
                return r;
            c = c->next;
        }
        a = a->next;
    }
    return S_OK;
}

int qpow(double *res, double x, int n) {
    bool rev = n < 0;
    if (rev)
        n *= -1;
    *res = 1;
    for (;;) {
        if (n & 1)
            (*res) *= x;
        n >>= 1;
        if (!n)
            break;
        x *= x;
    }
    if (rev)
        *res = 1 / *res;
    return 0;
}

typedef struct PolynomEvalArg {
    double *res;
    const Trie *vals;
} PolynomEvalArg;

int _polynom_eval_callback(const char *k, long v, void *ptr) {
    PolynomEvalArg *args = ptr;
    double *val = (double *)trie_get(args->vals, k);
    if (val == NULL) {
        fprintf(stderr, "ERROR: value for [%s] not provided\n", k);
        fflush(stderr);
        return S_INVALID_INPUT;
    }
    qpow(args->res, *val, v);
    return S_OK;
}

/// in trie pass pointers to double values
int polynom_eval(double *res, const Polynom *a, const Trie *vals) {
    *res = 0;
    while (a) {
        double r = 1;
        PolynomEvalArg args = {&r, vals};
        int s = trie_for_each(&a->cur.vars, _polynom_eval_callback, &args);
        if (s)
            return s;
        (*res) += r * a->cur.coef;
        a = a->next;
    }
    return S_OK;
}

/// var - by which variable to differentiate
int polynom_deriv(Polynom *p, const Polynom *a, const char *var) {
    while (a) {
        long val = trie_get(&a->cur.vars, var);
        if (val > 0) {
            Monom m = {0};
            m.coef = a->cur.coef * val;
            int r;
            if ((r = trie_dup(&m.vars, &a->cur.vars)) != S_OK ||
                (r = trie_set(&m.vars, var, val - 1)) != S_OK ||
                (r = polynom_add_monom(p, m)) != S_OK) {
                return r;
            }
        }
        a = a->next;
    }
    return S_OK;
}

/// var - by which variable to integrate
int polynom_prim(Polynom *p, const Polynom *a, const char *var) {
    // TODO:handle const value?
    while (a) {
        long val = trie_get(&a->cur.vars, var);
        Monom m = {0};
        m.coef = a->cur.coef / (val + 1);
        int r;
        if ((r = trie_dup(&m.vars, &a->cur.vars)) != S_OK ||
            (r = trie_set(&m.vars, var, val + 1)) != S_OK ||
            (r = polynom_add_monom(p, m)) != S_OK) {
            return r;
        }
        a = a->next;
    }
    return S_OK;
}

typedef struct PolynomGradArg {
    const Polynom *p;
    const char *var;
    Polynom *res;
} PolynomGradArg;

int _polynom_grad_callback(const char *k, long v, void *ptr) {
    (void)v;
    PolynomGradArg *arg = ptr;
    int n = strlen(k) + strlen(arg->var);

    char *s = malloc(n + 2);
    if (!s)
        return S_MALLOC;
    strcat(s, arg->var);
    strcat(s, "0");
    strcat(s, k);
    //
    Polynom cur = {0};
    int r = polynom_deriv(&cur, arg->p, k);

    Polynom *p = &cur;
    while (p) {
        if (trie_get(&p->cur.vars, s) != 0) {
            fprintf(stderr,
                    "ERROR: could not use [%s] as unary vector due to [%s]\n",
                    k, s);
            fflush(stderr);
            free(s);
            polynom_free(&cur);
            return S_INVALID_INPUT;
        }
        trie_set(&p->cur.vars, s, 1);
        p = p->next;
    }
    free(s);

    if (r)
        return r;
    Polynom res = {0};
    r = polynom_add(&res, arg->res, &cur);
    polynom_free(arg->res);
    polynom_free(&cur);

    *(arg->res) = res;
    return r;
}
/// var - name of singular vector
int polynom_grad(Polynom *p, const Polynom *a, const char *var) {
    while (a) {
        PolynomGradArg args = {.p = a, .res = p, .var = var};
        int r = trie_for_each(&a->cur.vars, _polynom_grad_callback, &args);
        if (r)
            return r;
        a = a->next;
    }
    return 0;
}
