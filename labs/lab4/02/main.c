#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

int is_sep(int c) {
    return isspace(c) || c == '(' || c == ')' || c == ',' || c == ';';
}

int parse_index(const char **s) {
    while (**s && isspace(**s))
        (*s)++;
    char c = tolower(**s);
    if ('a' <= c && c <= 'z') {
        (*s) += 2;
        return c - 'a';
    }
    return -1;
}

char *parse_str(const char **s) {
    while (**s && isspace(**s))
        (*s)++;
    const char *end = *s;
    while (*end && !is_sep(*end))
        end++;
    int n = end - *s;
    char *res = malloc(n + 1);
    if (!res)
        return NULL;
    memcpy(res, *s, n);
    for (int i = 0; i < n; i++) {
        res[i] = tolower(res[i]);
    }
    res[n] = 0;
    *s = end + 1;
    return res;
}

long parse_uint(const char **s) {
    while (**s && isspace(**s))
        (*s)++;
    char *ptr;
    size_t r = strtoul(*s, &ptr, 10);
    if (!is_sep(*ptr))
        return -1;
    *s = ptr + 1;
    return r;
}

typedef struct Arr {
    size_t size;
    unsigned *vals;
} Arr;

typedef int (*handle)(Arr *vars, const char **s);
#define ARRAY_CNT 26

typedef enum Status { S_OK, S_MALLOC, S_PARSE } Status;

int check_overflow(size_t r, Arr *a) {
    if (r >= a->size) {
        fflush(stdout);
        fprintf(stderr,
                "ERROR: array overflow: attempted to access index [%zu] of an "
                "array with lenth [%zu]\n",
                r, a->size);
        fflush(stderr);
    }
    return r >= a->size;
}

long parse_uint_from_file(const char **s) {
    while (**s && isspace(**s))
        (*s)++;
    if (**s == 0)
        return -1;
    char *ptr;
    size_t r = strtoul(*s, &ptr, 10);
    if (*ptr && !isspace(*ptr)) {
        *(ptr + 1) = 0;
        fprintf(stderr, "ERROR: failed to parse [%s] as number\n", *s);
        fflush(stderr);
        return -2;
    }
    *s = ptr + 1;
    return r;
}

int handle_load(Arr *vars, const char **s) {
    int index;
    char *path;
    if ((index = parse_index(s)) < 0 || *(*s - 1) != ',' ||  // first
        (path = parse_str(s)) == NULL || *(*s - 1) != ';' || // second
        (path != NULL && *path == 0)) {
        return S_PARSE;
    }
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "ERROR: failed to open file: [%s]\n", path);
        fflush(stderr);
        return S_OK;
    }
    free(path);

    size_t line_len = 0;
    char *line = NULL;

    size_t cap = 16;
    size_t size = 0;
    unsigned *arr = malloc(cap * sizeof(unsigned));
    if (!arr)
        return S_MALLOC;
    while (true) {
        int n = getline(&line, &line_len, f);
        if (n <= 1)
            break;
        const char *s = line;
        while (*s && isspace(*s))
            s++;
        while (true) {
            int parsed = parse_uint_from_file(&s);
            if (parsed == -2) {
                free(arr);
                free(line);
                fclose(f);
                return S_OK;
            }
            if (parsed == -1) {
                break;
            }
            if (size >= cap) {
                cap *= 2;
                unsigned *new_arr = realloc(arr, cap * sizeof(unsigned));
                if (!new_arr) {
                    free(line);
                    free(arr);
                    fclose(f);
                    return S_MALLOC;
                }
                arr = new_arr;
            }
            arr[size] = parsed;
            size++;
        }
    }
    Arr *a = &vars[index];
    if (a->size > 0)
        free(a->vals);
    a->vals = arr;
    a->size = size;

    free(line);
    fclose(f);
    return S_OK;
}
int handle_save(Arr *vars, const char **s) {
    int index;
    char *path;
    if ((index = parse_index(s)) < 0 || *(*s - 1) != ',' || // first
        (path = parse_str(s)) == NULL || *(*s - 1) != ';' ||
        (path != NULL && *path == 0)) // second
        return S_PARSE;
    FILE *f = fopen(path, "w");
    if (!f) {
        fprintf(stderr, "ERROR: failed to open file: [%s]\n", path);
        fflush(stderr);
        return S_OK;
    }
    free(path);
    Arr *a = &vars[index];
    for (size_t i = 0; i < a->size; i++) {
        fprintf(f, "%u ", a->vals[i]);
    }
    fflush(f);
    fclose(f);
    return 0;
}
int handle_rand(Arr *vars, const char **s) {
    int index, count, lb, rb;
    if ((index = parse_index(s)) < 0 || *(*s - 1) != ',' || // first
        (count = parse_uint(s)) < 0 || *(*s - 1) != ',' ||  // second
        (lb = parse_uint(s)) < 0 || *(*s - 1) != ',' ||     // third
        (rb = parse_uint(s)) < 0 || *(*s - 1) != ';' ||     // last
        rb < lb) {
        return S_PARSE;
    }
    Arr *a = &vars[index];
    a->vals = malloc(sizeof(unsigned) * count);
    if (!a->vals)
        return S_MALLOC;
    a->size = count;
    for (size_t i = 0; i < a->size; i++) {
        a->vals[i] = rand() % (rb - lb + 1) + lb;
    }
    return S_OK;
}
int handle_concat(Arr *vars, const char **s) {
    int i_a, i_b;
    if ((i_a = parse_index(s)) < 0 || *(*s - 1) != ',' || // first
        (i_b = parse_index(s)) < 0 || *(*s - 1) != ';') {
        return S_PARSE; // second
    }
    Arr *a = &vars[i_a];
    Arr *b = &vars[i_b];
    size_t new_size = a->size + b->size;
    unsigned *vals = realloc(a->vals, sizeof(unsigned) * new_size);
    if (!vals)
        return S_MALLOC;
    a->vals = vals;
    for (size_t i = 0; i < b->size; i++)
        a->vals[a->size + i] = b->vals[i];
    a->size = new_size;
    return 0;
}
int handle_free(Arr *vars, const char **s) {
    int index = parse_index(s);
    if (index < 0 || *(*s - 1) != ')' || **s != ';')
        return S_PARSE;
    Arr *a = &vars[index];
    if (a->size > 0) {
        a->size = 0;
        free(a->vals);
    }
    return S_OK;
}
int handle_remove(Arr *vars, const char **s) {
    int i_a, lb, n;
    if ((i_a = parse_index(s)) < 0 || *(*s - 1) != ',' || // first
        (lb = parse_uint(s)) < 0 || *(*s - 1) != ',' ||   // second
        (n = parse_uint(s)) < 0 || *(*s - 1) != ';') {
        return S_PARSE;
    }
    Arr *a = &vars[i_a];
    if (check_overflow(lb + n, a))
        return S_OK;
    for (int i = lb; i < lb + n; i++)
        a->vals[i] = a->vals[i + n];
    a->size -= n;
    return S_OK;
}
int handle_copy(Arr *vars, const char **s) {
    int i_a, lb, rb, i_b;
    if ((i_a = parse_index(s)) < 0 || *(*s - 1) != ',' || // first
        (lb = parse_uint(s)) < 0 || *(*s - 1) != ',' ||   // second
        (rb = parse_uint(s)) < 0 || *(*s - 1) != ',' ||   // third
        (i_b = parse_index(s)) < 0 || *(*s - 1) != ';' || // last
        rb < lb) {
        return S_PARSE;
    }
    Arr *a = &vars[i_a];
    if (check_overflow(rb, a))
        return S_OK;
    Arr *b = &vars[i_b];
    if (b->size > 0)
        free(b->vals);
    b->size = 0;
    b->size = rb - lb + 1;
    b->vals = malloc(b->size * sizeof(unsigned));
    if (!b->vals)
        return S_MALLOC;
    for (size_t i = 0; i < b->size; i++)
        b->vals[i] = a->vals[i + lb];
    return 0;
}

int cmp_inc(const void *a0, const void *b0) {
    unsigned a = *(unsigned *)a0;
    unsigned b = *(unsigned *)b0;
    return (a < b) ? -1 : a > b;
}

int cmp_dec(const void *a0, const void *b0) {
    return -cmp_inc(a0, b0);
}
int cmp_random(const void *a0, const void *b0) {
    (void)a0;
    (void)b0;
    return (rand() % 3) - 1;
}

int handle_sort(Arr *vars, const char **s) {
    while (isspace(**s))
        (*s)++;
    char c = tolower(**s);
    if (c < 'a' || 'z' < c)
        return S_PARSE;
    Arr *a = &vars[c - 'a'];
    (*s)++;
    if (**s != '+' && **s != '-')
        return S_PARSE;
    bool increase = **s == '+';
    (*s)++;
    if (**s != ';')
        return S_PARSE;
    if (increase) {
        qsort(a->vals, a->size, sizeof(unsigned), cmp_inc);
    } else {
        qsort(a->vals, a->size, sizeof(unsigned), cmp_dec);
    }

    return 0;
}
int handle_shuffle(Arr *vars, const char **s) {
    int index = parse_index(s);
    if (index < 0 || *(*s - 1) != ';')
        return S_PARSE;
    Arr *a = &vars[index];
    qsort(a->vals, a->size, sizeof(unsigned), cmp_random);
    return 0;
}
int handle_stats(Arr *vars, const char **s) {
    int index = parse_index(s);
    if (index < 0 || *(*s - 1) != ';')
        return S_PARSE;
    Arr *a = &vars[index];
    printf("%c:\n", index + 'A');
    printf("\tlen=%zu\n", a->size);
    if (a->size == 0)
        return S_OK;

    unsigned min = -1;
    unsigned max = 0;
    unsigned i_min = 0;
    unsigned i_max = 0;
    float mean = 0;
    for (size_t i = 0; i < a->size; i++) {
        unsigned val = a->vals[i];
        if (val < min) {
            i_min = i;
            min = val;
        }
        if (val > max) {
            i_max = i;
            max = val;
        }
        mean += a->vals[i];
    }
    mean /= a->size;

    printf("\tmin=%c[%d]=%u\n", 'A' + index, i_min, min);
    printf("\tmax=%c[%d]=%u\n", 'A' + index, i_max, max);

    unsigned *arr = malloc(a->size * sizeof(unsigned));
    if (!arr)
        return S_MALLOC;
    memcpy(arr, a->vals, a->size * sizeof(unsigned));
    qsort(arr, a->size, sizeof(unsigned), cmp_inc);

    unsigned len = 1;
    unsigned most_common_cnt = 1;
    unsigned most_common = arr[0];
    for (size_t i = 1; i < a->size; i++) {
        if (arr[i] == arr[i - 1]) {
            len++;
        } else {
            if (len >= most_common_cnt) {
                most_common_cnt = len;
                most_common = arr[i - 1];
            }
            len = 1;
        }
    }
    if (len >= most_common_cnt) {
        most_common_cnt = len;
        most_common = arr[a->size - 1];
    }
    printf("\tmost common=%u\n", most_common);
    printf("\tmean=%f\n", mean);
    float diff_first = mean - arr[0];
    float diff_last = arr[a->size - 1] - mean;
    float max_diff = diff_first > diff_last ? diff_first : diff_last;
    printf("\tmax deviation=%f\n", max_diff);
    fflush(stdout);
    free(arr);

    return 0;
}
int handle_print(Arr *vars, const char **s) {
    int index = parse_index(s);
    if (index < 0 || *(*s - 1) != ',')
        return S_PARSE;

    const char *s_copy = *s;
    char *arg2_str = parse_str(&s_copy);
    int arg_2 = parse_uint(s);

    if ((arg_2 < 0 || (*(*s - 1) != ',' && *(*s - 1) != ';')) &&
        (strcmp(arg2_str, "all") != 0 || *(s_copy - 1) != ';')) {
        return S_PARSE;
    }
    free(arg2_str);

    Arr *a = &vars[index];
    if (arg_2 >= 0 && *(*s - 1) == ';') { // print a, n;
        if (check_overflow(arg_2, a))
            return S_OK;
        printf("%c[%d] = %d\n", index + 'A', arg_2, a->vals[arg_2]);
        fflush(stdout);
        return S_OK;
    }

    int l, r;
    if (arg_2 < 0) { // print a, all;
        printf("%c = [", index + 'A');
        for (size_t i = 0; i < a->size; i++) {
            if (i > 0)
                printf(", ");
            printf("%d", a->vals[i]);
        }
        printf("]\n");
        fflush(stdout);
        return S_OK;
    }

    l = arg_2;
    r = parse_uint(s);
    if (r < 0 || *(*s - 1) != ';')
        return S_PARSE;

    if (check_overflow(r, a))
        return S_OK;
    printf("%c[%d..%d] = [", index + 'A', l, r);
    for (int i = l; i <= r; i++) {
        if (i > l)
            printf(", ");
        printf("%d", a->vals[i]);
    }
    printf("]\n");
    fflush(stdout);
    return S_OK;
}

int main(int argc, const char *argw[]) {
    FILE *f = stdin;
    if (argc > 1) {
        f = fopen(argw[1], "r");
        if (!f) {
            fprintf(stderr, "ERROR: failed to open file: [%s]\n", argw[1]);
            fflush(stderr);
            return 1;
        }
    }
    srand((unsigned int)time(NULL));
    size_t line_len = 0;
    char *line = NULL;
    const char *ops[] = {"load", "save", "rand",    "concat", "free", "remove",
                         "copy", "sort", "shuffle", "stats",  "print"};
    handle handles[] = {handle_load,   handle_save, handle_rand,
                        handle_concat, handle_free, handle_remove,
                        handle_copy,   handle_sort, handle_shuffle,
                        handle_stats,  handle_print};

    Arr arrays[ARRAY_CNT] = {0};
    while (true) {
        int n = getline(&line, &line_len, f);
        if (n <= 0)
            break;
        if (line[n - 1] == '\n') {
            n--;
            line[n] = 0;
        }
        const char *s = line;
        char *op = parse_str(&s);
        if (*op == 0)
            continue;
        handle *hand = NULL;
        for (size_t i = 0; i < sizeof(ops) / sizeof(ops[0]); i++) {
            if (strcmp(op, ops[i]) == 0) {
                hand = &handles[i];
                break;
            }
        }

        if (hand == NULL) {
            fprintf(stderr, "ERROR: unknown operation [%s]\n", op);
            fprintf(stderr, "supported operations: {");
            for (size_t i = 0; i < sizeof(ops) / sizeof(ops[0]); i++) {
                fprintf(stderr, "%s, ", ops[i]);
            }
            fprintf(stderr, "}\n");
            fflush(stderr);
            free(op);
            continue;
        }

        bool is_free = strcmp("free", op) == 0;
        free(op);

        if (is_free && *(s - 1) != '(') {
            fprintf(stderr, "ERROR: braces are expected as separator with "
                            "free command\n");
            fflush(stderr);
            continue;
        }

        if (!is_free && !isspace(*(s - 1))) {
            fprintf(stderr, "ERROR: commands must be followed by whitespace\n");
            fflush(stderr);
            continue;
        }

        int r = (*hand)(arrays, &s);
        if (r == S_MALLOC) {
            fprintf(stderr, "ERROR: buy more ram\n");
            return 1;
        }
        if (r == S_PARSE) {
            fprintf(stderr, "ERROR: failed to parse arguments from line [%s]\n",
                    line);
            fflush(stderr);
        }
    }
    for (size_t i = 0; i < ARRAY_CNT; i++) {
        if (arrays[i].size > 0)
            free(arrays[i].vals);
    }
    free(line);
    if (argc > 1)
        fclose(f);
    return 0;
}
