#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h> // TODO: remove me later lol

#include <preprocessor.h>
#include <tppalloc.h>

void pp_context_init(PP_Context *ctx) {
  ctx->include_depth = 0;
  ctx->current_file = NULL;
  ctx->include_paths.count = 0;
  ctx->include_paths.paths = NULL;
  ctx->emit_linemarkers = false;
}

void pp_context_add_include_path(PP_Context *ctx, const char *path) {
  PP_Include_Paths *ip = &ctx->include_paths;
  if(ip->count >= PP_MAX_INCLUDE_PATHS) return;

  if(ip->count == 0) {
    ip->paths = tpp_malloc(PP_MAX_INCLUDE_PATHS * sizeof(const char *));
  }

  ip->paths[ip->count++] = path;
}

static void pp_buf_init(PP_Buffer *buf) {
  buf->cap = 4096;
  buf->len = 0;
  buf->data = tpp_malloc(buf->cap);
}

static void pp_buf_append(PP_Buffer *buf, const char *data, size_t len) {
  if(buf->len + len >= buf->cap) {
    while(buf->len + len >= buf->cap)
      buf->cap *= 2;
    buf->data = tpp_realloc(buf->data, buf->cap);
  }
  memcpy(buf->data + buf->len, data, len);
  buf->len += len;
}

static void pp_buf_append_char(PP_Buffer *buf, char c) {
  pp_buf_append(buf, &c, 1);
}

static void pp_emit_linemarker(PP_Buffer *buf, int line, const char *file,
                               const char *flags) {
  char marker[512];
  int n;
  if(flags)
    n = snprintf(marker, sizeof(marker), "# %d \"%s\" %s\n", line, file, flags);
  else
    n = snprintf(marker, sizeof(marker), "# %d \"%s\"\n", line, file);
  if(n > 0) pp_buf_append(buf, marker, n);
}

void pp_buffer_free(PP_Buffer *buf) {
  if(buf->data) tpp_free(buf->data);
  buf->data = NULL;
  buf->len = 0;
  buf->cap = 0;
}

static char *read_entire_file(const char *path, size_t *out_len) {
  FILE *fp = fopen(path, "rb");
  if(!fp) return NULL;

  if(fseek(fp, 0L, SEEK_END) != 0) { fclose(fp); return NULL; }
  long file_size = ftell(fp);
  if(file_size < 0) { fclose(fp); return NULL; }
  rewind(fp);

  char *buf = tpp_malloc(file_size + 1);
  if(!buf) { fclose(fp); return NULL; }

  size_t n = fread(buf, 1, file_size, fp);
  if(n != (size_t)file_size) { tpp_free(buf); fclose(fp); return NULL; }

  buf[n] = '\0';
  if(out_len) *out_len = n;
  fclose(fp);
  return buf;
}

static char *splice_lines(const char *src, size_t len, size_t *out_len) {
  char *out = tpp_malloc(len + 1);
  size_t j = 0;

  for(size_t i = 0; i < len; ) {
    if(src[i] == '\\' && i + 1 < len && src[i + 1] == '\n') {
      i += 2;
    } else if(src[i] == '\\' && i + 2 < len && src[i + 1] == '\r' && src[i + 2] == '\n') {
      i += 3;
    } else {
      out[j++] = src[i++];
    }
  }

  out[j] = '\0';
  if(out_len) *out_len = j;
  return out;
}

static char *dir_of(const char *path) {
  char *copy = strdup(path);
  char *dir = strdup(dirname(copy));
  free(copy);
  return dir;
}

static char *try_resolve(const char *dir, const char *name) {
  size_t dlen = strlen(dir);
  size_t nlen = strlen(name);
  char *full = tpp_malloc(dlen + 1 + nlen + 1);

  memcpy(full, dir, dlen);
  full[dlen] = '/';
  memcpy(full + dlen + 1, name, nlen);
  full[dlen + 1 + nlen] = '\0';

  FILE *fp = fopen(full, "r");
  if(fp) { fclose(fp); return full; }
  tpp_free(full);
  return NULL;
}

static char *resolve_include(PP_Context *ctx, const char *name, bool is_quoted) {
  if(is_quoted && ctx->current_file) {
    char *dir = dir_of(ctx->current_file);
    char *resolved = try_resolve(dir, name);
    free(dir);
    if(resolved) return resolved;
  }

  for(size_t i = 0; i < ctx->include_paths.count; i++) {
    char *resolved = try_resolve(ctx->include_paths.paths[i], name);
    if(resolved) return resolved;
  }

  return NULL;
}

static bool pp_process_source(PP_Context *ctx, const char *src, size_t len, PP_Buffer *out);

static bool handle_include(PP_Context *ctx, const char *line, size_t line_len, PP_Buffer *out) {
  const char *p = line;
  const char *end = line + line_len;

  // skip whitespace after #include
  while(p < end && (*p == ' ' || *p == '\t'))
    p++;

  if(p >= end) {
    fprintf(stderr, "tpp: empty #include directive in '%s'\n", ctx->current_file);
    return false;
  }

  char delim_open, delim_close;
  bool is_quoted;

  if(*p == '"') {
    delim_open = '"'; delim_close = '"';
    is_quoted = true;
  } else if(*p == '<') {
    delim_open = '<'; delim_close = '>';
    is_quoted = false;
  } else {
    fprintf(stderr, "tpp: malformed #include in '%s'\n", ctx->current_file);
    return false;
  }

  (void)delim_open;
  p++;
  const char *name_start = p;

  while(p < end && *p != delim_close) p++;
  if(p >= end) {
    fprintf(stderr, "tpp: unterminated #include in '%s'\n", ctx->current_file);
    return false;
  }

  size_t name_len = p - name_start;
  char *name = tpp_malloc(name_len + 1);
  memcpy(name, name_start, name_len);
  name[name_len] = '\0';

  char *resolved = resolve_include(ctx, name, is_quoted);
  if(!resolved) {
    fprintf(stderr, "tpp: '%s' not found, included from '%s'\n",
            name, ctx->current_file);
    tpp_free(name);
    return false;
  }
  tpp_free(name);

  bool ok = pp_process_file(ctx, resolved, out);
  tpp_free(resolved);
  return ok;
}

// Skip over a line comment starting at //.
static size_t skip_line_comment(const char *src, size_t pos, size_t len) {
  while(pos < len && src[pos] != '\n')
    pos++;
  return pos;
}

// Skip over a block comment starting at /*.
static size_t skip_block_comment(const char *src, size_t pos, size_t len) {
  while(pos < len) {
    if(src[pos] == '*' && pos + 1 < len && src[pos + 1] == '/') return pos + 2;
    pos++;
  }
  return pos;
}

// Process already-loaded source text, expanding #include directives.
static bool pp_process_source(PP_Context *ctx, const char *src, size_t len, PP_Buffer *out) {
  size_t i = 0;
  int line_num = 1;

  while(i < len) {
    bool at_line_start = (i == 0 || src[i - 1] == '\n');

    if(at_line_start) {
      size_t save = i;

      // skip leading whitespace before '#'
      while(i < len && (src[i] == ' ' || src[i] == '\t'))
        i++;

      if(i < len && src[i] == '#') {
        i++;
        // skip whitespace between '#' and directive name
        while(i < len && (src[i] == ' ' || src[i] == '\t'))
          i++;

        if(i + 7 <= len && strncmp(src + i, "include", 7) == 0 &&
           (i + 7 >= len || src[i + 7] == ' ' || src[i + 7] == '\t' ||
            src[i + 7] == '"' || src[i + 7] == '<')) {
          i += 7;

          // find end of this line
          size_t line_start = i;
          while(i < len && src[i] != '\n') i++;
          size_t line_end = i;

          if(!handle_include(ctx, src + line_start, line_end - line_start, out))
            return false;

          // consume the newline
          if(i < len && src[i] == '\n') {
            line_num++;
            i++;
          }

          // emit return linemarker after coming back from the include
          if(ctx->emit_linemarkers)
            pp_emit_linemarker(out, line_num, ctx->current_file, "2");
          continue;
        }

        i = save;
      } else
        i = save;
    }

    if(src[i] == '/' && i + 1 < len && src[i + 1] == '/') {
      size_t end = skip_line_comment(src, i + 2, len);
      pp_buf_append(out, src + i, end - i);
      i = end;
    } else if(src[i] == '/' && i + 1 < len && src[i + 1] == '*') {
      size_t end = skip_block_comment(src, i + 2, len);
      pp_buf_append(out, src + i, end - i);
      i = end;
    } else if(src[i] == '"' || src[i] == '\'') {
      char quote = src[i];
      pp_buf_append_char(out, src[i]);
      i++;
      while(i < len && src[i] != quote) {
        if(src[i] == '\\' && i + 1 < len) {
          pp_buf_append_char(out, src[i]);
          i++;
        }
        pp_buf_append_char(out, src[i]);
        i++;
      }
      if(i < len) {
        pp_buf_append_char(out, src[i]);
        i++;
      }
    } else {
      if(src[i] == '\n') line_num++;
      pp_buf_append_char(out, src[i]);
      i++;
    }
  }

  return true;
}

bool pp_process_file(PP_Context *ctx, const char *path, PP_Buffer *out) {
  if(ctx->include_depth >= PP_MAX_INCLUDE_DEPTH) {
    fprintf(stderr, "tpp: #include depth limit (%d) exceeded at '%s'\n",
            PP_MAX_INCLUDE_DEPTH, path);
    return false;
  }

  size_t file_len;
  char *raw = read_entire_file(path, &file_len);
  if(!raw) {
    fprintf(stderr, "tpp: failed to read '%s'\n", path);
    return false;
  }

  size_t spliced_len;
  char *spliced = splice_lines(raw, file_len, &spliced_len);
  tpp_free(raw);

  const char *prev_file = ctx->current_file;
  ctx->current_file = path;
  ctx->include_depth++;

  if(!out->data) pp_buf_init(out);

  if(ctx->emit_linemarkers) {
    const char *flags = (ctx->include_depth > 1) ? "1" : NULL;
    pp_emit_linemarker(out, 1, path, flags);
  }

  bool ok = pp_process_source(ctx, spliced, spliced_len, out);

  ctx->include_depth--;
  ctx->current_file = prev_file;
  tpp_free(spliced);
  return ok;
}
