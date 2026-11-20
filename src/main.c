#include <stdio.h>

#include <libtpp.h>

static const char *usage =
  "Usage: tpp [options] file...\n"
  "Options:\n"
  ;

char *args_next(int *argc, char ***argv) {
  char *arg = **argv;
  *argv += 1;
  *argc -= 1;
  return arg;
}

int main(int argc, char **argv) {
  TPP_State *s = tpp_new();
  int ret = 0;
  
  args_next(&argc, &argv);
  if(argc == 0) { fprintf(stderr, usage); ret = 1; goto out; }
  while(argc > 0) {
    char *arg = args_next(&argc, &argv);
    if(*arg == '-') {}
    else {
      tpp_add_source_file(s, arg, FT_CPP);
    }
  }

  for(size_t i = 0; i < (da_hdr(s->source_files))->nb_elems; i++) {
    printf("file: %s\n", (((struct file **)s->source_files)[i])->name);
  }

 out:
  tpp_destroy(s);
  return ret;
}
