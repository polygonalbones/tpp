#define _POSIX_C_SOURCE 200809L
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <lexer.h>
#include <preprocessor.h>
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
static bool preprocess_only = false;
static PP_Context pp_ctx;

bool add_source_file(char *path) {
  const char *ext = strrchr(path, '.');
  if(!ext || ext == path) {
    printf("tpp: Invalid extension from file '%s'", path);
    return false;
  }

  if(strchr(ext, '/') || strchr(ext, '\\')) {
    printf("tpp: Invalid path '%s'", path);
    return false;
  }

  if(access(path, F_OK) != 0) {
    printf("tpp: Failed to access file '%s'", path);
    return false;
  }

  if(!(strcmp(ext, ".cpp") == 0 ||
       strcmp(ext, ".cc")  == 0 ||
       strcmp(ext, ".cxx") == 0)) {
    printf("tpp: Assuming '%s' is C++ source file", path);
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
  pp_context_init(&pp_ctx);

  args_next(&argc, &argv);
  while(argc > 0) {
    char *arg = args_next(&argc, &argv);
    if(*arg == '-') {
      if(strcmp(arg, "-dump-tokens") == 0) {
        dump_tokens = true;
        continue;
      }
      else if(strcmp(arg, "-I") == 0) {
        if(argc == 0) {
          fprintf(stderr, "tpp: -I requires an argument\n");
          rc = 1;
          goto tpp_exit;
        }
        pp_context_add_include_path(&pp_ctx, args_next(&argc, &argv));
      }
      else if(strncmp(arg, "-I", 2) == 0) {
        pp_context_add_include_path(&pp_ctx, arg + 2);
      }
      else if(strcmp(arg, "-E") == 0) {
        preprocess_only = true;
        continue;
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

  if(preprocess_only)
    pp_ctx.emit_linemarkers = true;
  
  for(size_t i = 0; i < source_files_len; i++) {
    char *source_file = source_files[i];

    PP_Buffer pp_buf = {0};
    if(!pp_process_file(&pp_ctx, source_file, &pp_buf)) {
      rc = 1;
      goto tpp_exit;
    }

    if(preprocess_only) {
      fwrite(pp_buf.data, 1, pp_buf.len, stdout);
      pp_buffer_free(&pp_buf);
      continue;
    }

    Lexer lexer = {0};
    lexer_from_buffer(&lexer, pp_buf.data, pp_buf.len);

    lexer_go(&lexer, dump_tokens);

    pp_buffer_free(&pp_buf);
  }

 tpp_exit:
  if(pp_ctx.include_paths.paths)
    tpp_free(pp_ctx.include_paths.paths);
  if(source_files_len > 0) tpp_free(source_files);
  return rc;
}
