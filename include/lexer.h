#ifndef LEXER_H_
#define LEXER_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <tppalloc.h>

typedef enum {
  LEXER_OK = 0,
} Lexer_Error;

typedef enum {
  TOKEN_UNKNOWN = 0,
  TOKEN_IDENTIFIER,
  TOKEN_KEYWORD,
  TOKEN_PP_NUMBER,
  TOKEN_EOF,
} Token_Kind;

typedef struct {
  Token_Kind kind;
  size_t start;
  size_t end;
} Token;

typedef struct {
  char *source;
  size_t source_len;
  size_t pos;
} Lexer;

bool lexer_globals_init();
bool lexer_from_file(Lexer *lexer, char *path);
bool lexer_go(Lexer *lexer, bool dump_tokens);

#endif // LEXER_H_
