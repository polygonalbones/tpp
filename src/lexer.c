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

static inline bool is_whitespace(char ch) {
  return ch == ' ' || ch == '\t' || ch == '\v' || ch == '\n' || ch == '\r' || ch == '\f';
}

static inline bool is_digit(char ch) {
  return (ch >= '0' && ch <= '9');
}

static inline bool is_nonzero_digit(char ch) {
  return (ch >= '1' && ch <= '9');
}

static inline bool is_nondigit(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

static inline bool is_hexadecimal_digit(char ch) {
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

static uint32_t hex_value(char ch) {
  if(ch >= '0' && ch <= '9') return ch - '0';
  if(ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  return ch - 'A' + 10;
}

static size_t scan_ucn(Lexer *lexer, size_t offset, uint32_t *value_out) {
  if(peek(lexer, offset) != '\\') return 0;

  char kind = peek(lexer, offset + 1);
  size_t n = (kind == 'u') ? 4 : (kind == 'U') ? 8 : 0;
  if(n == 0) return 0;

  uint32_t value = 0;
  for(size_t k = 0; k < n; k++) {
    char c = peek(lexer, offset + 2 + k);
    if(!is_hexadecimal_digit(c)) return 0;
    value = value * 16 + hex_value(c);
  }

  if(value_out) *value_out = value;
  return 2 + n;
}

static bool ucn_value_ok(uint32_t v) {
  if(v > 0x10FFFF) return false;
  else if(v >= 0xD800 && v <= 0xDFFF) return false;
  else if(v < 0xA0 && v != '$' && v != '@' && v != '`') return false;
  else return true;
}

static bool ucn_allowed_in_identifier(uint32_t v) {
  static const struct { uint32_t lo, hi; } ranges[] = {
    { 0x00A8, 0x00A8 }, { 0x00AA, 0x00AA }, { 0x00AD, 0x00AD }, { 0x00AF, 0x00AF },
    { 0x00B2, 0x00B5 }, { 0x00B7, 0x00BA }, { 0x00BC, 0x00BE }, { 0x00C0, 0x00D6 },
    { 0x00D8, 0x00F6 }, { 0x00F8, 0x00FF }, { 0x0100, 0x167F }, { 0x1681, 0x180D },
    { 0x180F, 0x1FFF }, { 0x200B, 0x200D }, { 0x202A, 0x202E }, { 0x203F, 0x2040 },
    { 0x2054, 0x2054 }, { 0x2060, 0x206F }, { 0x2070, 0x218F }, { 0x2460, 0x24FF },
    { 0x2776, 0x2793 }, { 0x2C00, 0x2DFF }, { 0x2E80, 0x2FFF }, { 0x3004, 0x3007 },
    { 0x3021, 0x302F }, { 0x3031, 0x303F }, { 0x3040, 0xD7FF }, { 0xF900, 0xFD3D },
    { 0xFD40, 0xFDCF }, { 0xFDF0, 0xFE44 }, { 0xFE47, 0xFFFD },
    { 0x10000, 0x1FFFD }, { 0x20000, 0x2FFFD }, { 0x30000, 0x3FFFD }, { 0x40000, 0x4FFFD },
    { 0x50000, 0x5FFFD }, { 0x60000, 0x6FFFD }, { 0x70000, 0x7FFFD }, { 0x80000, 0x8FFFD },
    { 0x90000, 0x9FFFD }, { 0xA0000, 0xAFFFD }, { 0xB0000, 0xBFFFD }, { 0xC0000, 0xCFFFD },
    { 0xD0000, 0xDFFFD }, { 0xE0000, 0xEFFFD },
  };
  for(size_t i = 0; i < sizeof(ranges)/sizeof(ranges[0]); i++)
    if(v >= ranges[i].lo && v <= ranges[i].hi) return true;
  return false;
}

static bool ucn_disallowed_initially(uint32_t v) {
  return (v >= 0x0300 && v <= 0x036F) || (v >= 0x1DC0 && v <= 0x1DFF) ||
         (v >= 0x20D0 && v <= 0x20FF) || (v >= 0xFE20 && v <= 0xFE2F);
}

static size_t ident_char_len(Lexer *lexer, bool first) {
  char c = current_char(lexer);
  if(is_nondigit(c)) return 1;
  if(!first && is_digit(c)) return 1;

  uint32_t v;
  size_t n = scan_ucn(lexer, 0, &v);
  if(n == 0) return 0;
  if(!ucn_value_ok(v) || !ucn_allowed_in_identifier(v)) return 0;
  if(first && ucn_disallowed_initially(v)) return 0;
  return n;
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
  else if(ident_char_len(lexer, true) > 0) {
    token->kind = TOKEN_IDENTIFIER;

    size_t n;
    while((n = ident_char_len(lexer, false)) > 0)
      advance(lexer, n);
    token->end = lexer->pos;

    size_t len = token->end - token->start;
    if(memchr(lexer->source + token->start, '\\', len) == NULL) {
      char buf[len + 1];
      memcpy(buf, lexer->source + token->start, len);
      buf[len] = '\0';
      if(set_contains(keywords_map, buf))
        token->kind = TOKEN_KEYWORD;
    }
  }
  else if(is_digit(current_char(lexer))
          || (current_char(lexer) == '.' && is_digit(peek(lexer, 1)))) {
    token->kind = TOKEN_PP_NUMBER;
    advance(lexer, is_digit(current_char(lexer)) ? 1 : 2);
    
    while(true) {
      char c = current_char(lexer);
      size_t n;

      if((c == 'e' || c == 'E') && (peek(lexer, 1) == '+' || peek(lexer, 1) == '-'))
        advance(lexer, 2);
      else if(c == '.')
        advance(lexer, 1);
      else if((n = ident_char_len(lexer, false)) > 0)
        advance(lexer, n);
      else if((unsigned char)c >= 0x80)
        advance(lexer, 1);
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
    if(err != LEXER_OK) goto eof_exit_loop;
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
