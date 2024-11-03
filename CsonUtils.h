#ifndef _CSON_UTILS_H
#define _CSON_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/stat.h>

#define BYTE_DELIMETER 1024.0

const char* BYTE_SUFFIXES[] = {"B", "KB", "MB", "GB", "TB"};

bool str_is_int(char* s);
bool str_is_float(char* s);

char* str_to_higher(char* s);
char* str_to_lower(char* s);

size_t str_size(char* s);

char* str_dup(char* s);

int str_char_index(char* s, char c);
void str_char_remove(char* s, char c);
void str_char_replace(char*s, char o, char n);
char* str_human_bytes(char* buffer, size_t buff_size, uint64_t bytes);

uint64_t file_size(char *file_path);

#define ANSI_RGB(r, g, b) ("\e[38;2;" #r ";" #g ";" #b "m")
#define ANSI_END "\e[0m"

// #define CSON_UTILS_IMPLEMENTATION // DEBUG
#ifdef CSON_UTILS_IMPLEMENTATION
bool str_is_int(char *s)
{
    if (!s) return false;
    char c;
    if (*s == '+' || *s == '-') s++;
    if (*s == '\0') return false;
    while ((c = *s++)){
        if (!isdigit(c)) return false;
    }
    return true;
}

bool str_is_float(char *s)
{
    char *ep = NULL;
    strtod(s, &ep);
    return (ep && !*ep);
}

int str_char_index(char *s, char c)
{
    if (!s) return -1;
    const char *p = s;
    do{
        if (*p==c) return p-s;
    } while (*++p != '\0');
    return -1;
}

void str_char_remove(char *s, char c)
{
    if (!s) return;
    char *pr = s, *pw = s;
    while (*pr != '\0'){
        *pw = *pr++;
        pw += (*pw != c);
    }
}

void str_char_replace(char *s, char o, char n)
{
    if (!s) return;
    char *p = s;
    while (*p != '\0'){
        if (*p == o) *p = n;
        p++;
    }
}

char* str_to_higher(char *s)
{
    char *w = s;
    do{
        if (0x60 < *w && *w < 0x7b) *w &= 0xdf;
    } while (*++w != '\0');
    return s;
}

char* str_to_lower(char *s)
{
    char *w = s;
    do{
        if (0x40 < *w && *w < 0x5b) *w |= 0x20;
    } while (*++w != '\0');
    return s;
}

size_t str_size(char *s)
{
    if (!s) return 0;
    char *r = s;
    while (*r++ != '\0'){}
    return r-s;
}

char* str_dup(char *s)
{
    if (!s) return NULL;
    char *d = (char*) calloc(str_size(s), sizeof(*d));
    if (d){
        strcpy(d, s);
    }
    return d;
}

void put_str(char *s)
{
    if (!s) return;
    char c;
    while ((c=*s++) != '\0'){
        putchar(c);
    }
}

char* str_human_bytes(char *buffer, size_t buff_size, uint64_t bytes)
{
    if (!buffer) return NULL;
    size_t suff_len = sizeof(BYTE_SUFFIXES)/sizeof(*BYTE_SUFFIXES);
    size_t i = 0;
    double dbl_bytes = bytes;
    if (bytes > BYTE_DELIMETER){
        while (bytes >= (int) BYTE_DELIMETER && i++ < suff_len-1){
            bytes /= (int) BYTE_DELIMETER;
            dbl_bytes = bytes;
        }
    }
    int n_p = snprintf(buffer, buff_size, "%.02lf %s", dbl_bytes, BYTE_SUFFIXES[i]);
    if (n_p < 0 || n_p >= buff_size) return NULL;
    return buffer;
}

uint64_t file_size(char *file_path)
{
    struct stat f_stats;
    if (stat(file_path, &f_stats) == -1) return 0;
    return (uint64_t) f_stats.st_size;
}

#endif // STRLIB_IMPLEMENTATION

#endif // _CSON_UTILS_H
