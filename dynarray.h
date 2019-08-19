#include "const.h"
#include <malloc.h>

typedef struct DynArray
{
    int count;
    int capacity;
    void **data;
} DynArray;

void dynarray_init(DynArray *parr, int initsize);

void dynarray_free(DynArray *parr);

void dynarray_realloc(DynArray * parr, int new_size);

void dynarray_add(DynArray *parr, void *data);

int dynarray_search(DynArray *parr, int key);
