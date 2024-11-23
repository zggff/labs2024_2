#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define HASHSIZE 128

int gen_hash(size_t *hash, const char *s) {
    *hash = 0;
    while (*s) {
        *hash = *hash * 62;
        if ('0' <= *s && *s <= '9')
            (*hash) += *s - '0';
        if ('A' <= *s && *s <= 'Z')
            (*hash) += *s - 'A' + 10;
        if ('a' <= *s && *s <= 'z')
            (*hash) += *s - 'a' + 36;
        s++;
    }
    return 0;
}

typedef struct HashElem {
    size_t hash;
    char *k;
    char *v;
} HashElem;

typedef struct HashList {
    size_t cap;
    size_t size;
    HashElem *vals;
} HashList;

int _hash_list_init(HashList *l) {
    l->cap = 4;
    l->size = 0;
    l->vals = malloc(l->cap * sizeof(HashElem));
    return !l->vals;
}

int _hash_list_push(HashList *l, HashElem e) {
    if (l->size >= l->cap) {
        size_t new_cap = l->cap *= 2;
        HashElem *new_vals = realloc(l->vals, new_cap * sizeof(HashElem));
        if (!new_vals)
            return 1;
        l->vals = new_vals;
        l->cap = new_cap;
    }
    l->vals[l->size] = e;
    l->size++;
    return 0;
}

typedef struct HashTable {
    size_t size;
    HashList *vals;
} HashTable;

int hash_init(HashTable *t, size_t size) {
    t->size = size;
    t->vals = malloc(t->size * sizeof(HashList));
    if (!t->vals)
        return 1;
    for (size_t i = 0; i < t->size; i++) {
        if (_hash_list_init(&t->vals[i]) != 0)
            return 1;
    }
    return 0;
}

int hash_free(HashTable *t) {
    for (size_t i = 0; i < t->size; i++) {
        if (t->vals[i].vals == NULL)
            continue;
        for (size_t j = 0; j < t->vals[i].size; j++) {
            HashElem e = t->vals[i].vals[j];
            free(e.k);
            free(e.v);
        }
        free(t->vals[i].vals);
    }
    free(t->vals);
    return 0;
}

int _hash_set(HashTable *t, HashElem e);

int _hash_rehash(HashTable *t) {
    size_t min = SIZE_MAX;
    size_t max = 0;
    for (size_t i = 0; i < t->size; i++) {
        if (t->vals[i].size < min)
            min = t->vals[i].size;
        if (t->vals[i].size > max)
            max = t->vals[i].size;
    }
    if (min == 0 || max < 2 * min)
        return 0;

    HashTable u = {0};
    if (hash_init(&u, t->size + HASHSIZE))
        return 0;

    for (size_t i = 0; i < t->size; i++) {
        for (size_t j = 0; j < t->vals[i].size; j++) {
            if (_hash_set(&u, t->vals[i].vals[j]))
                return 0;
        }
    }

    for (size_t i = 0; i < t->size; i++) 
        free(t->vals[i].vals);
    free(t->vals);
    
    *t = u;
    return 0;
}

int _hash_set(HashTable *t, HashElem e) {
    HashList *l = &t->vals[e.hash % t->size];
    for (size_t i = 0; i < l->size; i++) {
        if (strcmp(l->vals[i].k, e.k) == 0) {
            l->vals[i].v = e.v;
            return 0;
        }
    }
    return _hash_list_push(l, e) || _hash_rehash(t);
}

int hash_set(HashTable *t, const char *k, const char *v) {
    size_t hash;
    gen_hash(&hash, k);
    HashElem e = {0};
    size_t k_size = strlen(k) + 1;
    size_t v_size = strlen(v) + 1;
    e.hash = hash;
    e.k = malloc(k_size);
    e.v = malloc(v_size);
    if (e.k == NULL || e.v == NULL) {
        return 1;
    }
    memcpy(e.k, k, k_size);
    memcpy(e.v, v, v_size);
    return _hash_set(t, e);
}

const char *hash_get(const HashTable *t, const char *k) {
    size_t hash;
    gen_hash(&hash, k);
    HashList *l = &t->vals[hash % t->size];
    for (size_t i = 0; i < l->size; i++) {
        if (strcmp(l->vals[i].k, k) == 0) {
            return l->vals[i].v;
        }
    }
    return 0;
}

int hash_print(const HashTable *t) {
    for (size_t i = 0; i < t->size; i++) {
        for (size_t j = 0; j < t->vals[i].size; j++) {
            HashElem e = t->vals[i].vals[j];
            printf("[%s] : [%s]\n", e.k, e.v);
        }
    }
    return 0;
}

int main(int argc, char *argw[]) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: input file not provided\n");
        return 1;
    }
    FILE *f = fopen(argw[1], "r");
    if (!f) {
        fprintf(stderr, "ERROR: failed to open file: \"%s\"\n", argw[1]);
        return 1;
    }

    FILE *out;
    if (argc < 3) {
        out = stdout;
    } else {
        out = fopen(argw[2], "w");
        if (!out) {
            fprintf(stderr, "ERROR: failed to open file: \"%s\"\n", argw[2]);
            return 1;
        }
    }

    HashTable t = {0};
    hash_init(&t, HASHSIZE);

    size_t line_len = 0;
    char *line = NULL;
    while (true) {
        int n = getline(&line, &line_len, f);
        if (n <= 0)
            break;
        n--;
        line[n] = 0;
        char *s = line;
        while (isspace(*s))
            s++;

        if (strncmp("#define", s, 7) != 0) {
            if (*s != 0) {
                fprintf(stderr,
                        "ERROR: #define statements must be followed by "
                        "an empty line, not [%s]\n",
                        line);
                free(line);
                hash_free(&t);
                return 1;
            }
            break;
        }
        s += 7;
        if (*s && !isspace(*s)) {
            fprintf(stderr, "ERROR: unknown command: [%s]\n", line);
            free(line);
            hash_free(&t);
            return 1;
        }
        while (*s && isspace(*s)) {
            s++;
        }
        char *e = s;
        while (*e && !isspace(*e)) {
            e++;
        }
        *e = 0;
        e++;
        if (*s == 0) {
            fprintf(stderr, "ERROR: key must not be empty\n");
            free(line);
            hash_free(&t);
            return 1;
        }
        hash_set(&t, s, e);
    }
    while (true) {
        int n = getline(&line, &line_len, f);
        if (n <= 0)
            break;
        char *s = line;
        while (*s) {
            while (*s && !isalnum(*s)) {
                fputc(*s, out);
                s++;
            }
            char *e = s;
            char tmp;
            while (*e && isalnum(*e)) {
                e++;
            }
            tmp = *e;
            *e = 0;

            const char *val = hash_get(&t, s);
            if (val) {
                fprintf(out, "%s", val);
            } else {
                fprintf(out, "%s", s);
            }
            *e = tmp;
            s = e;
        }
    }
    free(line);

    hash_free(&t);

    fclose(f);
    return 0;
}
