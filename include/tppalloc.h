#ifndef TPP_ALLOC_H_
#define TPP_ALLOC_H_

#include <stdlib.h>

// macros that can be redefined which control
// how tpp allocates and frees memory.

// TODO: consider using rpmalloc on supported platforms

#ifndef tpp_malloc
#define tpp_malloc(size) malloc(size)
#endif

#ifndef tpp_realloc
#define tpp_realloc(ptr, new_size) realloc(ptr, new_size)
#endif

#ifndef tpp_calloc
#define tpp_calloc(num, size) calloc(num, size)
#endif

#ifndef tpp_free
#define tpp_free(ptr) free(ptr)
#endif

#endif // TPP_ALLOC_H_
