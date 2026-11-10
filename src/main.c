#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <tppalloc.h>

char *args_next(int *argc, char ***argv) {
  char *arg = **argv;
  *argc -= 1;
  *argv += 1;
  return arg;
}

static char **source_files = NULL;
static size_t source_files_len = 0;

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
    }
    else if(!add_source_file(arg)) {
      return 1;
    }
  }
  return 0;
}
