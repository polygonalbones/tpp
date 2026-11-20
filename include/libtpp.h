#ifndef LIB_TPP_H_
#define LIB_TPP_H_

#include <stdbool.h>

struct da_hdr {
  size_t nb_elems;
};

struct file {
  char type;
  char name[1];
};

/* sfile->type */
#define FT_NONE 0
#define FT_CPP  1

typedef struct {
  /* C++ language options */
  unsigned int cpp_ver; /* default is 201103 for C++11 */
  bool ms_ext;          /* enable MSVC extensions */
  bool gnu_ext;         /* enable GNU extensions */
  bool tpp_ext;         /* enable TPP extensions */

  char **source_files;
  char **include_paths;
} TPP_State;

/* TPP_State->cpp_ver */
#define CPPVER_11 201103

#define da_hdr(da) ((struct da_hdr *)(da) - 1)
#define da_nb(da) ((da_hdr(da))->nb_elems)
#define da_append(da, elem) do { \
    void *_da = *(da); \
    _da_append(&_da, (elem)); \
    *(da) = _da; \
  } while(0)
#define da_free(da, cb) do { \
    void *_da = *(da); \
    _da_free(&_da, (cb)); \
    *(da) = _da; \
  } while(0)
void _da_append(void **da, void *elem);
void _da_free(void **da, void (*free_cb)(void *));

TPP_State *tpp_new();
void tpp_add_source_file(TPP_State *s, char *name, int type);

void tpp_destroy(TPP_State *s);

#endif /* LIB_TPP_H_ */
