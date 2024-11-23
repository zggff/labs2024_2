#include "tokenize.h"
#include <stdio.h>

char token_str_getter(char *buf, int *off, void *f) {
    (void)f;
    if (buf[*off] == 0)
        return 0;
    char res = buf[*off];
    (*off)++;
    return res;
}

char token_file_getter(char buf[BUF_SIZE], int *off, void *f_ptr) {
    FILE *f = f_ptr;
    if (*buf == 0 || *off >= BUF_SIZE) {
        *off = 0;
        long c = fread(buf, 1, BUF_SIZE, f);
        if (c < BUF_SIZE) {
            buf[c] = 0;
        }
        if (c == 0) {
            return 0;
        }
    }
    char res = buf[*off];
    if (res)
        (*off)++;
    return res;
}

int _is_sep(char c) {
    return isspace(c) || c == '-' || c == '+' || c == '*' || c == '^' ||
           c == ',' || c == ';' || c == '(' || c == ')' || c == ':' ||
           c == '{' || c == '}' || c == '%' || c == '[';
}

long token_get(char **s, size_t *cap, char *buf, int *off,
               token_char_getter get, void *f) {
    if (*s == NULL) {
        *cap = 32;
        *s = malloc(*cap);
        if (*s == NULL)
            return 0;
    }
    char c;
    do {
        c = get(buf, off, f);
    } while (c && isspace(c));

    if (c == '%') {
        do {
            c = get(buf, off, f);
        } while (c && c != '\n');
        do {
            c = get(buf, off, f);
        } while (c && isspace(c));
    }

    if (c == '[') {
        do {
            c = get(buf, off, f);
        } while (c && c != ']');
        do {
            c = get(buf, off, f);
        } while (c && isspace(c));
    }

    if (!c) {
        (*s)[0] = 0;
        return 0;
    }

    size_t i = 0;
    do {
        if (i >= *cap - 1) {
            size_t new_cap = *cap * 2;
            char *s2 = realloc(*s, new_cap);
            if (s2 == NULL) {
                free(s);
                return 0;
            }
            *cap = new_cap;
            *s = s2;
        }
        (*s)[i] = c;
        i++;
        c = get(buf, off, f);
    } while (c && !_is_sep(c) && !_is_sep((*s)[i - 1]));
    if (c != 0)
        (*off)--;

    (*s)[i] = 0;
    return i;
}

long token_parse_list(char ***s, size_t *s_cap, char *buf, int *off,
                      token_char_getter get, void *f) {
    if (*s == NULL) {
        *s_cap = 32;
        *s = malloc(*s_cap * sizeof(char *));
        if (*s == NULL)
            return 0;
        memset(*s, 0, *s_cap * sizeof(char *));
    }

    size_t i = 0;
    size_t tok_len = 0;
    char *tok = NULL;
    while (true) {
        if (i >= *s_cap - 1) {
            size_t new_cap = *s_cap * 2;
            char **s2 = realloc(*s, new_cap * sizeof(char *));
            if (s2 == NULL) {
                free(s);
                return 0;
            }
            *s = s2;
            memset(*s + *s_cap, 0, (new_cap - *s_cap) * sizeof(char *));
            *s_cap = new_cap;
        }

        int n = token_get(&tok, &tok_len, buf, off, get, f);
        if (n <= 0)
            break;
        (*s)[i] = malloc(n + 1);
        memcpy((*s)[i], tok, n + 1);
        i++;
        if (strcmp(tok, ";") == 0)
            break;
    }
    free(tok);
    return i;
}

int token_parse_file(char ***s, size_t *s_cap, char buffer[BUF_SIZE], int *off,
                     FILE *f) {
    return token_parse_list(s, s_cap, buffer, off, token_file_getter, f);
}

int token_parse_str(char ***res, size_t *res_cap, const char *str) {
    int off = 0;
    int n = strlen(str);
    char *s = malloc(n + 1);
    if (!s)
        return -1;
    memcpy(s, str, n);
    int r = token_parse_list(res, res_cap, s, &off, token_str_getter, NULL);
    free(s);
    return r;
}

int token_print_error(FILE *f, const char *tok, const char *exp) {
    if (tok)
        fprintf(f, "ERROR: expected [%s] got [%s]\n", exp, tok);
    else
        fprintf(f, "ERROR: expected [%s] got none\n", exp);
    fflush(f);
    return 0;
}

int token_print(FILE *f, char **tok) {
    fprintf(f, "{");
    for (size_t i = 0; tok[i]; i++) {
        if (i > 0)
            fprintf(f, " ");
        fprintf(f, "[%s]", tok[i]);
    }
    fprintf(f, "}\n");
    return 0;
}
