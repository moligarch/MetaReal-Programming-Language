/**
 * @file error.c
 * This file contains the file definitions of the "error.h" file.
*/

#include <error/error.h>

void mr_illegal_chr_print(mr_illegal_chr_t *error, mr_str_ct fname, mr_str_ct code)
{
    // dummy
    if (!code)
        return;

    if (error->expected)
        fprintf(stderr, "\nExpected Character Error: '%c'\n", error->chr);
    else
        fprintf(stderr, "\nIllegal Character Error: '%c'\n", error->chr);

    fprintf(stderr, "File \"%s\", line %llu\n\n", fname, error->pos.ln);
}
