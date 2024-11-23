#include <ctype.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define VAR_CNT 26
#define BUF_SIZE 128

typedef unsigned char vec;
#define VEC_MAX 255
#define VEC_LEN 8

char _getc(char buf[BUF_SIZE], int *off, FILE *f) {
    if (*buf == 0) {
        *off = 0;
        long c = fread(buf, 1, BUF_SIZE, f);
        if (c < BUF_SIZE)
            buf[c] = 0;
        if (c == 0)
            return 0;
    }
    if (*off >= BUF_SIZE) {
        *off = 1;
        buf[0] = buf[BUF_SIZE - 1];
        long c = fread(buf + 1, 1, BUF_SIZE - 1, f);
        if (*off + c < BUF_SIZE)
            buf[*off + c] = 0;
        if (c == 0)
            return 0;
    }
    char res = buf[*off];
    (*off)++;
    return res;
}

int is_sep(char c) {
    return c == ',' || c == ';' || c == '(' || c == ')';
}

int is_token(const char *s) {
    const char *tokens[] = {":=", "(", ")", ",", ";"};
    for (size_t i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++) {
        if (strncmp(s, tokens[i], strlen(tokens[i])) == 0)
            return 1;
    }
    return 0;
}

long get_token(char **s, size_t *cap, char buf[BUF_SIZE], int *off, FILE *f) {
    if (*s == NULL) {
        *cap = 32;
        *s = malloc(*cap);
        if (*s == NULL)
            return 0;
    }
    size_t i = 0;
    char c;
    do {
        c = _getc(buf, off, f);
    } while (c && isspace(c));

    if (c == '%') {
        do {
            c = _getc(buf, off, f);
        } while (c && c != '\n');
        do {
            c = _getc(buf, off, f);
        } while (c && isspace(c));
    }

    if (c == '{') {
        do {
            c = _getc(buf, off, f);
        } while (c && c != '}');
        do {
            c = _getc(buf, off, f);
        } while (c && isspace(c));
    }

    if (!c) {
        (*s)[0] = 0;
        return 0;
    }

    bool num = isalnum(c);
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
        (*s)[i] = tolower(c);
        i++;
        c = _getc(buf, off, f);
    } while (c && !isspace(c) && !is_token(*s) && isalnum(c) == num &&
             c != '{' && c != '%');
    (*off)--;

    (*s)[i] = 0;
    return i;
}

#define MAX_CMD_LEN 20

long tokenize(char *s[MAX_CMD_LEN], char buf[BUF_SIZE], int *off, FILE *f) {
    size_t i = 0;

    size_t tok_len = 0;
    char *tok = NULL;
    while (i < MAX_CMD_LEN) {
        int n = get_token(&tok, &tok_len, buf, off, f);
        if (n <= 0)
            break;
        s[i] = malloc(n + 1);
        memcpy(s[i], tok, n + 1);
        i++;
        if (strcmp(tok, ";") == 0)
            break;
    }
    free(tok);
    return i;
}

int print_error(FILE *f, const char *tok, const char *exp) {
    if (*tok)
        fprintf(f, "ERROR: expected [%s] got [%s]\n", exp, tok);
    else
        fprintf(f, "ERROR: expected [%s] got none\n", exp);
    fflush(f);
    return 0;
}

int print_tokens(FILE *f, char *tok[MAX_CMD_LEN]) {
    fprintf(f, "{");
    for (size_t i = 0; i < MAX_CMD_LEN && tok[i]; i++) {
        if (i > 0)
            fprintf(f, " ");
        fprintf(f, "[%s]", tok[i]);
    }
    fprintf(f, "}\n");
    return 0;
}

int is_number(const char *s) {
    for (const char *c = s; *c; c++) {
        if (!isnumber(*c)) {
            return 0;
        }
    }
    return 1;
}

#define FREE_TOKS()                                                            \
    {                                                                          \
        for (size_t i = 0; i < MAX_CMD_LEN; i++) {                             \
            if (toks[i]) {                                                     \
                free(toks[i]);                                                 \
                toks[i] = NULL;                                                \
            }                                                                  \
        }                                                                      \
    }

typedef enum TryStatus {
    TryOk = 0,
    TryNotMatch = 1,
    TryError = 2,
} TryStatus;

typedef TryStatus (*try)(FILE *f, bool trace, size_t n, char *toks[MAX_CMD_LEN],
                         vec vars[VAR_CNT]);

TryStatus try_read(FILE *f, size_t base, vec *res) {
    char *line = NULL;
    size_t line_len;
    size_t n = getline(&line, &line_len, stdin);
    if (n == 0) {
        fprintf(f, "ERROR: failed to read from stdin\n");
        return TryError;
    }
    char *ptr;
    size_t c = strtoul(line, &ptr, base);
    if (*ptr != 0 && !isspace(*ptr)) {
        fprintf(f, "ERROR: failed to parse [%s] as number in base [%zu]\n",
                line, base);
        return TryError;
    }
    if (c > VEC_MAX) {
        fprintf(f,
                "ERROR: number [%zu] is out of bounds for vec. Max vec value "
                "[%d]\n",
                c, VEC_MAX);
        return TryError;
    }
    *res = c;
    return TryOk;
}

TryStatus try_write(FILE *f, size_t base, vec num, char id) {
    bool print_base = base >= 2;
    if (base == 0)
        base = 2;
    if (num == 0 && print_base) {
        fprintf(f, "%c_%zu = 0\n", id, base);
        fflush(stdout);
        return TryOk;
    }
    char str[128];
    int i = 0;
    while (num > 0) {
        char digit = num % base;
        digit = digit < 10 ? '0' + digit : 'A' + digit - 10;
        str[i] = digit;
        num /= base;
        i++;
    }
    if (!print_base) {
        while (i < VEC_LEN) {
            str[i] = '0';
            i++;
        }
    }
    str[i] = 0;
    for (int j = 0; j < i / 2; j++) {
        char tmp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = tmp;
    }
    if (print_base)
        fprintf(f, "%c_%zu = %s\n", id, base, str);
    else
        fprintf(f, "%c = %s\n", id, str);
    fflush(stdout);
    return TryOk;
}

TryStatus try_read_write(FILE *f, bool trace, size_t n, char *toks[MAX_CMD_LEN],
                         vec vars[VAR_CNT]) {
    if (strcmp(toks[0], "read") != 0 && strcmp(toks[0], "write") != 0)
        return TryNotMatch;
    if (n <= 1 || strcmp(toks[1], "(") != 0) {
        print_error(f, toks[1], "(");
        return TryError;
    }
    if (n <= 2 || !isalpha(toks[2][0]) || toks[2][1] != 0) {
        print_error(f, toks[2], "<name>");
        return TryError;
    }
    int id = toks[2][0] - 'a';
    if (n <= 2 || strcmp(toks[3], ",") != 0) {
        print_error(f, toks[3], ",");
        return TryError;
    }
    char *ptr;
    if (n <= 4) {
        print_error(f, toks[4], "<base>");
        return TryError;
    }
    size_t base = strtoul(toks[4], &ptr, 10);
    if (*ptr) {
        fprintf(f, "ERROR: failed to parse [%s] as number\n", toks[4]);
        fflush(f);
        return TryError;
    }
    if (base < 2 || base > 36) {
        fprintf(f, "ERROR: base [%zu] not in range [2..36]\n", base);
        fflush(f);
        return TryError;
    }
    if (n <= 5 || strcmp(toks[5], ")") != 0) {
        print_error(f, toks[1], ")");
        return TryError;
    }
    if (strcmp(toks[0], "read") == 0) {
        vec old = vars[id];
        TryStatus r = try_read(f, base, &vars[id]);
        if (trace) {
            fprintf(f, "read(%c, %zu);\n", id + 'A', base);
            fprintf(f, "\t");
            try_write(f, 0, old, id + 'A');
            fprintf(f, "\t->\n");
            fprintf(f, "\t");
            try_write(f, 0, vars[id], id + 'A');
        }
        return r;
    }
    if (trace) {
        fprintf(f, "write(%c, %zu);\n\t", id + 'A', base);
        try_write(f, 0, vars[id], id + 'A');
    }

    return try_write(stdout, base, vars[id], 'A' + id);
}

TryStatus try_negation(FILE *f, bool trace, size_t n, char *toks[MAX_CMD_LEN],
                       vec vars[VAR_CNT]) {
    if (!isalpha(toks[0][0]) || toks[0][1] != 0) {
        print_error(f, toks[0], "<res_id>");
        return TryError;
    }
    int res_id = toks[0][0] - 'a';
    if (n <= 1 || strcmp(toks[1], ":=") != 0) {
        print_error(f, toks[1], ":=");
        return TryError;
    }
    if (n <= 2 || strcmp(toks[2], "\\") != 0) {
        return TryNotMatch;
    }
    if (!isalpha(toks[3][0]) || toks[3][1] != 0) {
        print_error(f, toks[3], "<id>");
        return TryError;
    }
    int id = toks[3][0] - 'a';
    if (trace) {
        fprintf(f, "%c := \\%c;\n\t", res_id + 'A', id + 'A');
        try_write(f, 0, vars[res_id], res_id + 'A');
        fprintf(f, "\t");
        try_write(f, 0, vars[id], id + 'A');
    }
    vars[res_id] = ~vars[id];
    if (trace) {
        fprintf(f, "\t->\n\t");
        try_write(f, 0, vars[res_id], res_id + 'A');
    }

    return TryOk;
}

TryStatus try_op(FILE *f, bool trace, size_t n, char *toks[MAX_CMD_LEN],
                 vec vars[VAR_CNT]) {
    if (!isalpha(toks[0][0]) || toks[0][1] != 0) {
        print_error(f, toks[0], "<res_id>");
        return TryError;
    }
    if (n <= 1 || strcmp(toks[1], ":=") != 0) {
        print_error(f, toks[1], ":=");
        return TryError;
    }
    if (n <= 2 || !isalpha(toks[2][0]) || toks[2][1] != 0) {
        print_error(f, toks[2], "<id_a>");
        return TryNotMatch;
    }
    if (n <= 3) {
        print_error(f, toks[3], "<op>");
        return TryError;
    }
    if (n <= 4 || !isalpha(toks[4][0]) || toks[4][1] != 0) {
        print_error(f, toks[4], "<id_b>");
        return TryError;
    }
    int res_id = toks[0][0] - 'a';
    int id_a = toks[2][0] - 'a';
    int id_b = toks[4][0] - 'a';
    vec *res = &vars[res_id];
    vec res_old = *res;
    vec a = vars[id_a];
    vec b = vars[id_b];

    if (strcmp("+", toks[3]) == 0) {
        *res = a | b;
    } else if (strcmp("&", toks[3]) == 0) {
        *res = a & b;
    } else if (strcmp("->", toks[3]) == 0) {
        *res = (~a) | b;
    } else if (strcmp("<-", toks[3]) == 0) {
        *res = (~b) | a;
    } else if (strcmp("~", toks[3]) == 0) {
        *res = ~(a ^ b);
    } else if (strcmp("<>", toks[3]) == 0) {
        *res = a ^ b;
    } else if (strcmp("+>", toks[3]) == 0) {
        *res = ~(a | (~b));
    } else if (strcmp("?", toks[3]) == 0) {
        *res = ~(a & b);
    } else if (strcmp("!", toks[3]) == 0) {
        *res = ~(a | b);
    } else {
        print_error(f, toks[3], "<op>");
        return TryError;
    }

    if (trace) {
        fprintf(f, "%c := %c %s %c;\n\t", res_id + 'A', id_a + 'A', toks[3],
                id_b + 'A');
        try_write(f, 0, res_old, res_id + 'A');
        fprintf(f, "\t");
        try_write(f, 0, vars[id_a], id_a + 'A');
        fprintf(f, "\t");
        try_write(f, 0, vars[id_b], id_b + 'A');
        fprintf(f, "\t->\n\t");
        try_write(f, 0, vars[res_id], res_id + 'A');
    }

    return TryOk;
}

int main(int argc, const char *argw[]) {
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

    bool trace = false;
    if (argc > 2 && strcmp(argw[2], "/trace") == 0)
        trace = true;

    FILE *out = stdout;
    if (trace && argc > 3) {
        out = fopen(argw[3], "w");
        if (!out) {
            fprintf(stderr, "ERROR: failed to open file: [%s]\n", argw[1]);
            fflush(stderr);
            return 1;
        }
    }

    char buf[BUF_SIZE] = {0};
    int off = 0;

    char *toks[MAX_CMD_LEN] = {0};

    vec vars[VAR_CNT] = {0};

    try tries[] = {try_read_write, try_negation, try_op};

    while (true) {
        size_t n = tokenize(toks, buf, &off, f);
        if (n <= 0)
            break;
        // print_tokens(stdout, toks);
        // FREE_TOKS();
        // continue;
        if (n < 5) {
            fprintf(stderr, "ERROR: not enough tokens:\n\t");
            print_tokens(stderr, toks);
            return 2;
        }
        if (n > 7) {
            fprintf(stderr, "ERROR: too many tokens:\n\t");
            print_tokens(stderr, toks);
            return 2;
        }
        for (size_t i = 0; i < sizeof(tries) / sizeof(tries[0]); i++) {
            int r = (tries[i])(out, trace, n, toks, vars);
            if (r == TryOk)
                break;
            if (r == TryError)
                return 2;
        }
        FREE_TOKS();
    }

    fclose(f);
    return 0;
}
