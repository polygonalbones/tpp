#ifndef PREPROCESSOR_H_
#define PREPROCESSOR_H_

#include <stdbool.h>
#include <stddef.h>

#define PP_MAX_INCLUDE_DEPTH 256
#define PP_MAX_INCLUDE_PATHS 64

typedef struct {
  char *data;
  size_t len;
  size_t cap;
} PP_Buffer;

typedef struct {
  const char **paths;
  size_t count;
} PP_Include_Paths;

typedef struct {
  PP_Include_Paths include_paths;
  int include_depth;
  const char *current_file;
  bool emit_linemarkers;
} PP_Context;

bool pp_process_file(PP_Context *ctx, const char *path, PP_Buffer *out);
void pp_context_init(PP_Context *ctx);
void pp_context_add_include_path(PP_Context *ctx, const char *path);
void pp_buffer_free(PP_Buffer *buf);

#endif // PREPROCESSOR_H_
