#ifndef LEXER_H_
#define LEXER_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <tppalloc.h>

typedef enum {
  TOKEN_UNKNOWN = 0,
  TOKEN_IDENTIFIER,
  TOKEN_KEYWORD,
  TOKEN_INT_LITERAL,
  TOKEN_CHAR_LITERAL,
  TOKEN_FLOAT_LITERAL,
  TOKEN_STRING_LITERAL,
  TOKEN_BOOL_LITERAL,
  TOKEN_PTR_LITERAL,
  TOKEN_USERDEF_LITERAL,
  TOKEN_OP,
  TOKEN_PUNCTUATOR,
  TOKEN_EOF,
} Token_Kind;

typedef struct {
  Token_Kind kind;
  size_t start;
  size_t end;
} Token;

typedef struct {
  char *source;
  size_t pos;
} Lexer;

bool lexer_globals_init();
bool lexer_from_file(Lexer *lexer, char *path);
bool lexer_go(Lexer *lexer);

#endif // LEXER_H_
