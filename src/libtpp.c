#include <stdlib.h>
#include <string.h>

#include <libtpp.h>

void _da_append(void **da, void *elem) {
  struct da_hdr *hdr;
  size_t nb;

  if(*da) {
    hdr = da_hdr(*da);
    nb = hdr->nb_elems;
  }
  else {
    hdr = NULL;
    nb = 0;
  }

  if((nb & (nb - 1)) == 0) {
    hdr = realloc(hdr, sizeof(*hdr) + (nb ? nb * 2 : 1) * sizeof(void *));
    hdr->nb_elems = nb;
    *da = hdr + 1;
  }
  ((void **)*da)[nb] = elem;
  hdr->nb_elems = nb + 1;
}

void _da_free(void **da, void (*free_cb)(void *)) {
  if(!*da) return;
  struct da_hdr *hdr = da_hdr(*da);
  if(free_cb) {
    void **elems = *da;
    for(size_t i = 0; i < hdr->nb_elems; i++)
      free_cb(elems[i]);
  }
  free(hdr);
  *da = NULL;
}

TPP_State *tpp_new() {
  size_t size = sizeof(TPP_State);
  TPP_State *s = malloc(size);
  
  memset(s, 0, size);
  s->cpp_ver = CPPVER_11;
  s->ms_ext = false;
  s->gnu_ext = false;
  s->tpp_ext = true;
  return s;
}

void tpp_add_source_file(TPP_State *s, char *name, int type) {
  struct file *f = malloc(sizeof *f + strlen(name));
  f->type = type;
  strcpy(f->name, name);
  da_append(&s->source_files, f);
}

void tpp_destroy(TPP_State *s) {
  da_free(&s->source_files, free);
  free(s);
}
