#ifndef COMPILER_UTIL_H
#define COMPILER_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdlib.h>
#include <stdio.h>

/* Dynamic storage allocation wrappers */
void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size);
void Free(void *ptr);

/* Standard I/O wrappers */
void Fclose(FILE *fp);
FILE *Fdopen(int fd, const char *type);
char *Fgets(char *ptr, int n, FILE *stream);
FILE *Fopen(const char *filename, const char *mode);
void Fputs(const char *ptr, FILE *stream);
size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);


#ifdef __cplusplus
};
#endif // __cplusplus


#endif //COMPILER_UTIL_H
