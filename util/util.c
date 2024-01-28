#include <util.h>

/**********************************************************************************************************************
                                            Dynamic storage allocation wrappers
 **********************************************************************************************************************/

void *Malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void *Realloc(void *ptr, size_t size) {
    ptr = realloc(ptr, size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: realloc failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void Free(void *ptr) {
    free(ptr);
}

/**********************************************************************************************************************
                                                Standard I/O wrappers
 **********************************************************************************************************************/

void Fclose(FILE *fp) {
    if (fclose(fp) != 0) {
        fprintf(stderr, "Error: fclose failed\n");
        exit(EXIT_FAILURE);
    }
}

FILE *Fdopen(int fd, const char *type) {
    FILE *fp = fdopen(fd, type);
    if (fp == NULL) {
        fprintf(stderr, "Error: fdopen failed\n");
        exit(EXIT_FAILURE);
    }
    return fp;
}

char *Fgets(char *ptr, int n, FILE *stream) {
    char *rptr = fgets(ptr, n, stream);
    if (ferror(stream)) {
        fprintf(stderr, "Error: fgets failed\n");
        exit(EXIT_FAILURE);
    }
    return rptr;
}

FILE *Fopen(const char *filename, const char *mode) {
    FILE *fp = fopen(filename, mode);
    if (fp == NULL) {
        fprintf(stderr, "Error: fopen failed\n");
        exit(EXIT_FAILURE);
    }
    return fp;
}

void Fputs(const char *ptr, FILE *stream) {
    if (fputs(ptr, stream) == EOF) {
        fprintf(stderr, "Error: fputs failed\n");
        exit(EXIT_FAILURE);
    }
}

size_t Fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t n = fread(ptr, size, nmemb, stream);
    if (ferror(stream)) {
        fprintf(stderr, "Error: fread failed\n");
        exit(EXIT_FAILURE);
    }
    return n;
}

void Fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    if (fwrite(ptr, size, nmemb, stream) != nmemb) {
        fprintf(stderr, "Error: fwrite failed\n");
        exit(EXIT_FAILURE);
    }
}

