#pragma once
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define TRIE_SIZE 36 // alphabet

typedef enum Status {
    S_OK = 0,
    S_INVALID_INPUT = 1,
    S_MALLOC = 2,
} Status;

typedef struct Trie Trie;
struct Trie {
    Trie *children[TRIE_SIZE];
    long val; // we can hold any 4 byte or less value including doubles
};

typedef int (*trie_callback)(const char *var, long val, void *ptr);

int trie_set(Trie *t, const char *k, long v);
long trie_get(const Trie *t, const char *k);
int trie_free(Trie *t);
int trie_for_each(const Trie *t, trie_callback call, void *ptr);
int trie_eq(const Trie *a, const Trie *b);
int trie_len(const Trie *t);
int trie_dup(Trie *r, const Trie *t);
