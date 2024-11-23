#pragma once

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128

typedef char (*token_char_getter)(char *, int *, void *);

char token_str_getter(char *buf, int *off, void *f);
char token_file_getter(char buf[BUF_SIZE], int *off, void *f_ptr);

long token_get(char **s, size_t *cap, char *buf, int *off,
               token_char_getter get, void *f);
long token_parse_list(char ***s, size_t *s_cap, char *buf, int *off,
                      token_char_getter get, void *f);
int token_parse_file(char ***s, size_t *s_cap, char buffer[BUF_SIZE], int *off,
                     FILE *f);
int token_parse_str(char ***res, size_t *res_cap, const char *str);
int token_print_error(FILE *f, const char *tok, const char *exp);
int token_print(FILE *f, char **tok);
