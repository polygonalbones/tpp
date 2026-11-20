#ifndef LIB_TPP_H_
#define LIB_TPP_H_

#include <stdbool.h>

typedef void * da_t;

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

  da_t source_files;
  da_t include_paths;
} TPP_State;

/* TPP_State->cpp_ver */
#define CPPVER_11 201103

#define da_hdr(da) ((da) ? (struct da_hdr *)(da) - 1 : NULL)
#define da_len(da) ((da) ? da_hdr(da)->nb_elems : 0)
void da_append(da_t *da, void *elem);
void da_free(da_t *da, void (*free_cb)(void *));

TPP_State *tpp_new();
void tpp_add_source_file(TPP_State *s, char *name, int type);

void tpp_destroy(TPP_State *s);

#endif /* LIB_TPP_H_ */
