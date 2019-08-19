#include <malloc.h>
#include "const.h"

typedef struct DynString
{
    /* data */
    int count;
    int capacity;
    char *data;
} DynString;

void dynstring_init(DynString *pstr, int initsize);

void dynstring_free(DynString * pstr);

void dynstring_reset(DynString * pstr);

void dynstring_realloc(DynString * pstr, int new_size);

void dynstring_chcat(DynString *pstr, int ch);
