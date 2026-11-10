#ifndef TPP_ALLOC_H_
#define TPP_ALLOC_H_

// TODO: consider using rpmalloc on supported platforms

#include <stdlib.h>

#ifndef tpp_malloc
#define tpp_malloc(size) malloc(size)
#endif

#ifndef tpp_realloc
#define tpp_realloc(ptr, new_size) realloc(ptr, new_size)
#endif

#ifndef tpp_free
#define tpp_free(ptr) free(ptr)
#endif

#endif // TPP_ALLOC_H_
