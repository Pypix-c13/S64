#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>

char *reader(const char *source) {
    FILE *fptr = fopen(source, "r");
    if(!fptr) return NULL;

    fseek(fptr, 0, SEEK_END);
    long size = ftell(fptr);
    rewind(fptr);

    char *buffer = (char *)malloc(size + 1);
    if(!buffer) {
        printf("Failed to allocate memory!.\n");
        fclose(fptr);
        return NULL;
    }

    size_t read = fread(buffer, 1, size, fptr);
    buffer[read] = '\0';

    fclose(fptr);
    return buffer;
}

bool is_file(const char *source) {
    FILE *fptr = fopen(source, "r");
    if(!fptr) {
        printf("File not found!.\n");
        return false;
    }

    struct stat path_stat;
    if(stat(source, &path_stat) != 0) return false;

    if(!S_ISREG(path_stat.st_mode)) {
        printf("Its not a file!.\n");
        return false;
    }

    const char *dot = strrchr(source, '.');
    if(!dot || strcmp(dot, ".sbit") != 0) {
        printf("Extension must be .sbit");
        return false;
    }

    fclose(fptr);
    return true;
}