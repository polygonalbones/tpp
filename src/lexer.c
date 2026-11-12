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
  lexer->source_len = bytes_read;
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
  return (ch >= '0' && ch <= '9');
}

static bool is_nonzero_digit(char ch) {
  return (ch >= '1' && ch <= '9');
}

static bool is_nondigit(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

static bool is_hexadecimal_digit(char ch) {
  return
    (ch >= '0' && ch <= '9') ||
    (ch >= 'a' && ch <= 'f') ||
    (ch >= 'A' && ch <= 'F');
}

const char *fmt_token_kind(Token_Kind kind) {
  // -Wswitch and -Werror=switch ensure I don't miss a case.
  switch(kind) {
  case TOKEN_IDENTIFIER:
    return "identifier";
  case TOKEN_KEYWORD:
    return "keyword";
  case TOKEN_PP_NUMBER:
    return "pp_number";
  case TOKEN_EOF:
    return "eof";
  case TOKEN_UNKNOWN:
    return "unknown";
  }
}

static hmap_t *keywords_map = NULL;

// keywords present in c++11
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

const char current_char(Lexer *lexer) {
  if(lexer->pos >= lexer->source_len)
    return '\0';
  return lexer->source[lexer->pos];
}

const char peek(Lexer *lexer, size_t offset) {
  size_t p = lexer->pos + offset;
  if(p >= lexer->source_len)
    return '\0';
  return lexer->source[p];
}

inline void advance(Lexer *lexer, size_t steps) {
  lexer->pos += ((lexer->pos + steps) >= lexer->source_len
                 ? lexer->source_len
                 : steps);
}

Lexer_Error lexer_next(Lexer *lexer, Token *token) {
  while(is_whitespace(current_char(lexer))) {
    advance(lexer, 1);
  }
  
  *token = (Token){
    .kind = TOKEN_UNKNOWN,
    .start = lexer->pos,
    .end = lexer->pos,
  };

  if(current_char(lexer) == '\0') {
    token->kind = TOKEN_EOF;
    return LEXER_OK;
  }
  else if(is_nondigit(current_char(lexer))) {
    size_t buf_len = 1;
    token->kind = TOKEN_IDENTIFIER;
    advance(lexer, 1);
    while(is_nondigit(current_char(lexer)) || is_digit(current_char(lexer))) {
      advance(lexer, 1);
      buf_len += 1;
    }
    token->end = lexer->pos;

    // look up token value in keyword set
    char buf[buf_len + 1];
    memcpy(buf, lexer->source + token->start, buf_len);
    buf[buf_len] = '\0';
    if(set_contains(keywords_map, buf))
      token->kind = TOKEN_KEYWORD;
  }
  else if(is_digit(current_char(lexer))
          || (current_char(lexer) == '.' && is_digit(peek(lexer, 1)))) {
    token->kind = TOKEN_PP_NUMBER;

    advance(lexer, is_digit(current_char(lexer)) ? 1 : 2);

    while(true) {
      char c = current_char(lexer);

      if((c == 'e' || c == 'E') && (peek(lexer, 1) == '+' || peek(lexer, 1) == '-'))
        advance(lexer, 2);
      else if(is_digit(c) || is_nondigit(c) || c == '.')
        advance(lexer, 1);
      else if(c == '\\' && (peek(lexer, 1) == 'u' || peek(lexer, 1) == 'U')) {
        size_t n = (peek(lexer, 1) == 'u') ? 4 : 8;
        size_t k = 0;
        while(k < n && is_hexadecimal_digit(peek(lexer, 2 + k)))
          k++;
        if(k != n) break;
        advance(lexer, 2 + n);
      }
      else if((unsigned char)c >= 0x80) {
        advance(lexer, 1);
      }
      else
        break;
    }

    token->end = lexer->pos;
  }
  return LEXER_OK;
}

bool lexer_go(Lexer *lexer, bool dump_tokens) {
  Token token = {0};
  while(true) {
    Lexer_Error err = lexer_next(lexer, &token);
    if(dump_tokens)
      printf("%s '%.*s'\n",
             fmt_token_kind(token.kind),
             (int)(token.end - token.start),
             lexer->source + token.start);
    
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
