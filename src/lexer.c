#include <hmap.h>
#include <hmap_err.h>
#include <lexer.h>

bool lexer_from_file(Lexer *lexer, char *path) {
  FILE *fp = fopen(path, "rb");
  if(fp == NULL) return false;

  if(fseek(fp, 0L, SEEK_END) != 0) return false;

  long file_size = ftell(fp);
  if(file_size < 0) {
    fclose(fp);
    return false;
  }

  rewind(fp);
  
  lexer->source = (char*)tpp_malloc(file_size + 1);
  if(lexer->source == NULL) {
    fclose(fp);
    return false;
  }

  size_t bytes_read = fread(lexer->source, 1, file_size, fp);
  if(bytes_read != (size_t)file_size) {
    tpp_free(lexer->source);
    fclose(fp);
    return false;
  }

  lexer->source[bytes_read] = '\0';
  fclose(fp);
  return true;
}

int set_contains(hmap_t *map, const char *key) {
  void *dummy_val = NULL;
  err_t *err = hmap_slookup(map, key, &dummy_val);
  if(err == ERR_OK) return 1;

  if(err->code == HMAP_ERR_NOTFOUND) {
    err_dispose(err);
    return 0;
  }

  return 0;
}

static bool is_whitespace(char ch) {
  return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\n' || ch == '\r' || ch == '\f';
}

static bool is_digit(char ch) {
  return ch >= '0' && ch <= '9';
}

static bool is_nondigit(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

const char *fmt_token_kind(Token_Kind kind) {
  // -Wswitch and -Werror=switch ensure I don't miss a case.
  switch(kind) {
  case TOKEN_IDENTIFIER:
    return "identifier";
  case TOKEN_KEYWORD:
    return "keyword";
  case TOKEN_INT_LITERAL:
    return "int_literal";
  case TOKEN_CHAR_LITERAL:
    return "char_literal";
  case TOKEN_FLOAT_LITERAL:
    return "float_literal";
  case TOKEN_STRING_LITERAL:
    return "string_literal";
  case TOKEN_BOOL_LITERAL:
    return "bool_literal";
  case TOKEN_PTR_LITERAL:
    return "ptr_literal";
  case TOKEN_USERDEF_LITERAL:
    return "userdef_literal";
  case TOKEN_OP:
    return "operator";
  case TOKEN_PUNCTUATOR:
    return "punctuator";
  case TOKEN_EOF:
    return "eof";
  case TOKEN_UNKNOWN:
    return "unknown";
  }
}

static hmap_t *keywords_map = NULL;

#define BASE_KEYWORDS \
  "alignas", "continue", "friend", "register", "true", \
  "alignof", "decltype", "goto", "reinterpret_cast", "try", \
  "asm", "default", "if", "return", "typedef" \
  "auto", "delete", "inline", "short", "typeid", \
  "bool", "do", "int", "signed", "typename", \
  "break", "double", "long", "sizeof", "union", \
  "case", "dynamic_cast", "mutable", "static", "unsigned", \
  "catch", "else", "namespace", "static_assert", "using", \
  "char", "enum", "new", "static_cast", "virtual", \
  "char16_t", "explicit", "noexcept", "struct", "void", \
  "char32_t", "export", "nullptr", "switch", "volatile", \
  "class", "extern", "operator", "template", "wchar_t", \
  "const", "false", "private", "this", "while", \
  "constexpr", "float", "protected", "thread_local", \
  "const_cast", "for", "public", "throw"

#define CREATE_KEYWORD_MAP(keywords) do { \
    const char *const _temp_kw[] = { keywords }; \
    size_t kw_size = sizeof(_temp_kw) / sizeof(_temp_kw[0]); \
    hmap_create((&keywords_map), kw_size); \
    for(size_t i = 0; i < kw_size; i++) { \
      hmap_swrite((keywords_map), _temp_kw[i], NULL); \
    } \
  } while(0)

bool lexer_globals_init() {
  ERR_F err = ERR_OK;
  CREATE_KEYWORD_MAP(BASE_KEYWORDS);
  if(err != ERR_OK) {
    fprintf(stderr, "failed to create keywords map\n");
    return false;
  }
  return true;
}

#define ch lexer->source[lexer->pos]

Token lexer_next(Lexer *lexer) {
  while(is_whitespace(ch)) {
    lexer->pos++;
  }
  
  Token token = {
    .kind = TOKEN_UNKNOWN,
    .start = lexer->pos,
    .end = lexer->pos,
  };

  if(ch == '\0') {
    token.kind = TOKEN_EOF;
    return token;
  }

  if(is_nondigit(ch)) {
    size_t buf_len = 1;
    token.kind = TOKEN_IDENTIFIER;
    lexer->pos++;
    while(is_nondigit(ch) || is_digit(ch)) {
      lexer->pos++;
      buf_len += 1;
    }
    token.end = lexer->pos;

    char buf[buf_len + 1];
    memcpy(buf, lexer->source + token.start, buf_len);
    buf[buf_len] = '\0';
    if(set_contains(keywords_map, buf))
      token.kind = TOKEN_KEYWORD;
  }
  return token;
}

#undef ch
#undef slice

bool lexer_go(Lexer *lexer) {
  while(true) {
    Token token = lexer_next(lexer);
    printf("%s '%.*s'\n",
           fmt_token_kind(token.kind),
           (int)(token.end - token.start), lexer->source + token.start);
    switch(token.kind) {
    case TOKEN_EOF:
      goto eof_exit_loop;
    case TOKEN_UNKNOWN:
      fprintf(stderr, "tpp: encountered unknown token\n");
      return false;
    default:
      break;
    }
  }
 eof_exit_loop:
  return true;
}
