#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <lexer.h>
#include <tppalloc.h>

char *args_next(int *argc, char ***argv) {
  char *arg = **argv;
  *argc -= 1;
  *argv += 1;
  return arg;
}

// Global compiler state
static int rc = 0;
static char **source_files = NULL;
static size_t source_files_len = 0;
static bool dump_tokens = false;

bool add_source_file(char *path) {
  const char *ext = strrchr(path, '.');
  if(!ext || ext == path) {
    printf("Invalid extension from file '%s'", path);
    return false;
  }

  if(strchr(ext, '/') || strchr(ext, '\\')) {
    printf("Invalid path '%s'", path);
    return false;
  }

  if(access(path, F_OK) != 0) {
    printf("Failed to access file '%s'", path);
    return false;
  }

  if(!(strcmp(ext, ".cpp") == 0 ||
       strcmp(ext, ".cc")  == 0 ||
       strcmp(ext, ".cxx") == 0)) {
    printf("Assuming '%s' is C++ source file", path);
  }
  
  int il = source_files_len;
  if((il & (il - 1)) == 0) {
    int cap = (il == 0) ? 1 : il * 2;
    
    char **new_source_files = tpp_realloc(source_files, cap * sizeof(char *));
    source_files = new_source_files;
  }

  source_files[il] = strdup(path);
  source_files_len = il + 1;
  return true;
}

int main(int argc, char **argv) {
  args_next(&argc, &argv);
  while(argc > 0) {
    char *arg = args_next(&argc, &argv);
    if(*arg == '-') {
      if(strcmp(arg, "-dump-tokens") == 0) {
        dump_tokens = true;
      }
      else {
        fprintf(stderr, "tpp: invalid flag '%s'\n", arg);
        rc = 1;
        goto tpp_exit;
      }
    }
    else if(!add_source_file(arg)) {
      rc = 1;
      goto tpp_exit;
    }
  }

  if(source_files_len == 0) {
    rc = 1;
    goto tpp_exit;
  }

  // only call me once
  if(!lexer_globals_init()) {
    rc = 1;
    goto tpp_exit;
  }
  for(size_t i = 0; i < source_files_len; i++) {
    char *source_file = source_files[i];

    Lexer lexer = {0};
    if(!lexer_from_file(&lexer, source_file)) {
      rc = 1;
      goto tpp_exit;
    }

    lexer_go(&lexer, dump_tokens);
    
    tpp_free(lexer.source);
  }

 tpp_exit:
  if(source_files_len > 0) tpp_free(source_files);
  return rc;
}
