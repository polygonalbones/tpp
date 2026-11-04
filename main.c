#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
#define TPP_ARCH_X86_64
#else
#error "unsupported arch"
#endif

#if defined(__linux__)
#define TPP_OS_LINUX
#else
#error "unsupported operating system"
#endif

#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Redefinable macros for common tpp functions
#ifndef tpp_malloc
#define tpp_malloc(x) malloc(x)
#endif

#ifndef tpp_realloc
#define tpp_realloc(x, y) realloc(x, y)
#endif

#ifndef tpp_warn
#define tpp_warn(fmt, ...) fprintf(stderr, "tpp: warning: "fmt"\n", __VA_ARGS__)
#endif

#ifndef tpp_error
#define tpp_error(fmt, ...) fprintf(stderr, "tpp: error: "fmt"\n", __VA_ARGS__)
#endif

#include "lexer.c"

typedef enum {
  CPP_98,
  // below are placeholders lol
  CPP_03,
  CPP_11,
  CPP_14,
  CPP_17,
  CPP_20,
  CPP_23,
  CPP_26,
} Cpp_Standard;

static struct {
  char **inputs;
  int inputs_len;
  Cpp_Standard std;
} *TPP_State = NULL;

char *args_next(int *argc, char ***argv) {
  char *arg = **argv;
  *argv += 1;
  *argc -= 1;
  return arg;
}

bool inputs_add(char *path) {
  const char *ext = strrchr(path, '.');
  if(!ext || ext == path) {
    tpp_error("Invalid extension from file '%s'", path);
    return false;
  }

  if(strchr(ext, '/') || strchr(ext, '\\')) {
    tpp_error("Invalid path '%s'", path);
    return false;
  }

  if(access(path, F_OK) != 0) {
    tpp_error("Failed to access file '%s'", path);
    return false;
  }

  if(!(strcmp(ext, ".cpp") == 0 ||
       strcmp(ext, ".cc")  == 0 ||
       strcmp(ext, ".cxx") == 0)) {
    tpp_warn("Assuming '%s' is C++ source file", path);
  }
  
  int il = TPP_State->inputs_len;
  if((il & (il - 1)) == 0) {
    int cap = (il == 0) ? 1 : il * 2;
    
    char **new_inputs = tpp_realloc(TPP_State->inputs, cap * sizeof(char *));
    TPP_State->inputs = new_inputs;
  }

  TPP_State->inputs[il] = strdup(path);
  TPP_State->inputs_len = il + 1;
  return true;
}

bool go_compile() {
  Lexer lex = {0};
  for(size_t i = 0; i < TPP_State->inputs_len; i++) {
    char *path = TPP_State->inputs[i];

    FILE *fp = fopen(path, "r");
    if(!fp) {
      tpp_error("Failed to open file '%s'", path);
      return false;
    }

    if(fseek(fp, 0L, SEEK_END) != 0) {
      fclose(fp);
      return false;
    }

    long file_size = ftell(fp);
    if(file_size < 0) {
      fclose(fp);
      return false;
    }

    // rewind
    if(fseek(fp, 0L, SEEK_SET) != 0) {
      fclose(fp);
      return false;
    }

    lex.source = tpp_malloc(file_size + 1);
    size_t br = fread(lex.source, 1, file_size, fp);
    if(br < (size_t)file_size && ferror(fp)) {
      free(lex.source);
      fclose(fp);
      return false;
    }
    lex.source[br] = '\0';

    if(!lexer_tokenize(&lex)) {
      free(lex.source);
      fclose(fp);
      return false;
    }

    free(lex.source);
    fclose(fp);
  }
  return true;
}

int main(int argc, char **argv) {
  TPP_State = tpp_malloc(sizeof(*TPP_State));
  TPP_State->inputs = NULL;
  TPP_State->inputs_len = 0;
  
  args_next(&argc, &argv);
  while(argc > 0) {
    char *arg = args_next(&argc, &argv);
    if(*arg == '-') {
      char *flag = arg + 1;
      // TODO: -D, -o, -Wall, -Wextra, -x
      if(strncmp(flag, "std", 3) == 0) {
        // TODO :p
      }
      else {
        tpp_error("Unknown argument '%s'", arg);
        free(TPP_State->inputs);
        free(TPP_State);
        return 1;
      }
    }
    else if(!inputs_add(arg)) {
      free(TPP_State->inputs);
      free(TPP_State);
      return 1;
    }
  }

  if(!go_compile()) {
    free(TPP_State->inputs);
    free(TPP_State);
    return 1;
  }

  free(TPP_State->inputs);
  free(TPP_State);
  return 0;
}
