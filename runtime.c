#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

long tigermain(long a);

void* checked_malloc(long len) 
{
    void *p = malloc(len);
    assert(p);
    return p;
}

long *initArray(long size, long init)
{
    long i;
    long* a = (long*)malloc(size * sizeof(long));
    for (i = 0; i < size; i++) {
        a[i] = init;
    }
    return a;
}

long *allocRecord(long size)
{
    long i;
    long *p, *a;
    p = a = (long *)malloc(size);
    for (i = 0; i < size; i += sizeof(long)) {
        *p++ = 0;
    }
    return a;
}

long stringEqual(char* s, char* t)
{
    if (strcmp(s,t)) {
        return 0;
    } else {
        return 1;
    }
}

void print(char* s)
{
    printf(s);
}

void printi(long k)
{
	printf("%d", k);
}

void flush()
{
    fflush(stdout);
}

char consts[256][2] = {0};
char empty = {0};

long main()
{
    long i;
    for (i = 0; i < 256; i++) {
        consts[i][0] = i;
    }
    return tigermain(0 /* static link */);
}

long ord(char* s)
{
    if (s[0]) {
        return s[0];
    } else {
        return -1;
    }
}

char *chr(long i)
{
    if (i < 0 || i >= 256) {
        printf("chr(%d) out of range\n", i);
        exit(1);
    }
    return (char*)&consts[i];
}

long size(char *s)
{
    return strlen(s);
}

char* substring(char *s, long first, long n)
{
    long length = strlen(s);
    if (first < 0 || first + n > length) {
        printf("substring([%d],%d,%d) out of range\n", length, first, n);
        exit(1);
    }
    if (n == 1) {
        return (char*)&consts[first];
    }
    char* t = (char*)malloc(n + 1);
    long i;
    for (i = 0; i < n; i++) {
        t[i] = s[first + i];
    }
    t[n] = 0;
    return t;
}

char* concat(char* a, char* b)
{
    long len_a = strlen(a);
    if (len_a == 0) {
        return b;
    }
    long len_b = strlen(b);
    if (len_b == 0) {
        return a;
    } 
    long i, n = len_a + len_b;
    char* t = (char*)malloc(n + 1);
    for (i = 0; i < len_a; i++)
        t[i] = a[i];
    for (i = 0; i < len_b; i++)
        t[i + len_a] = b[i];
    return t;
}

long not(long i)
{
    return !i;
}

#undef getchar

char *__wrap_getchar()
{
    long i = getc(stdin);
    if (i == EOF) {
        return &empty;
    } else {
        return (char*)&consts[i];
    }
}
