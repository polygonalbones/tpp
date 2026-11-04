typedef struct {
  char *source;
  size_t cursor;
} Lexer;

typedef enum {
  TOKEN_EOF = 0,
  TOKEN_IDENT,
} Token_Kind;

typedef struct {
  Token_Kind kind;
  char *value;
  size_t value_len;
} Token;

inline char peek(Lexer *lexer) {
  return lexer->source[lexer->cursor];
}

char advance(Lexer *lexer) {
  char c = peek(lexer);
  if(c != '\0') {
    lexer->cursor++;
  }
  return c;
}

Token next_tok(Lexer *lexer) {
  while(isspace(peek(lexer))) advance(lexer);
  
  Token token;
  token.value = &lexer->source[lexer->cursor];
  token.value_len = 0;

  char ch = peek(lexer);
  if(ch == '\0') {
    token.kind = TOKEN_EOF;
    return token;
  }

  if(isalpha(ch)) {
    token.kind = TOKEN_IDENT;
    while(isalnum(peek(lexer))) advance(lexer);
    token.value_len = &lexer->source[lexer->cursor] - token.value;
    printf("kind: %d value: %.*s\n", token.kind, token.value_len, token.value);
    return token;
  }

  advance(lexer);
  return token;
}

bool lexer_tokenize(Lexer *lexer) {
  lexer->cursor = 0;
  Token token;
  do {
    token = next_tok(lexer);
  }
  while(token.kind != TOKEN_EOF);
  return true;
}
