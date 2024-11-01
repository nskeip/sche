#ifndef _SCHE_LIB_H
#define _SCHE_LIB_H

#include <stddef.h>

typedef enum {
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_NAME,
  TOKEN_TYPE_PAR_OPEN,
  TOKEN_TYPE_PAR_CLOSE,
} TokenType;

typedef union {
  const char *s;
  long num;
} Value;

typedef struct {
  TokenType type;
  Value value;
} Token;

typedef enum {
  TOKENIZER_SUCCESS,
  TOKENIZER_ERROR_TOO_MANY_TOKENS,
  TOKENIZER_ERROR_INVALID_NAME,
  TOKENIZER_ERROR_MEMORY_ALLOC,
} TokenizerStatus;

typedef struct {
  size_t tokens_n;
  Token *tokens;
} TokenList;

typedef struct {
  TokenizerStatus status;
  TokenList token_list;
} TokenizerResult;

typedef enum {
  EXPR_TYPE_NAME,
  EXPR_TYPE_INT,
  EXPR_TYPE_SUBEXPR,
} ExpressionType;

typedef struct Expression {
  ExpressionType type;
  union {
    Value value;
    struct Expression *subexpr;
  };
  struct Expression *next;
} Expression;

typedef enum {
  PARSE_SUCCESS,
  PARSE_ERROR_TOO_SHORT_EXPR,
  PARSE_ERROR_UNBALANCED_PAR,
} ParserResultType;

typedef struct {
  ParserResultType type;
  Expression *expr;
} ParserResult;

typedef enum {
  EVAL_SUCCESS,
  EVAL_ERROR_TOKENIZATION,
  EVAL_ERROR_PARSING,
  EVAL_ERROR_NAME_EXPECTED,
  EVAL_ERROR_NAME_OR_SUBEXPR_EXPECTED,
  EVAL_ERROR_MEMORY_ALLOC,
  EVAL_ERROR_WRONG_ARGS_N,
  EVAL_ERROR_UNDEFINED_FUNCTION,
} EvalResultType;

typedef struct {
  EvalResultType type;
  union {
    TokenizerStatus tokenizer_error;
    ParserResultType parser_error;
    Value value;
  };
} EvalResult;

typedef struct {
  const char *name;
  const int min_args_n;
  const int max_args_n;
  long (*const run)(size_t args_n, const long *args);
} Function;

typedef void *(*allocator)(size_t);
typedef void *(deallocator)(void *);

TokenizerResult tokenize_with_allocator(const char *, allocator);
TokenizerResult tokenize(const char *s);
ParserResult parse_with_allocator(size_t, const Token[], allocator);
ParserResult parse(size_t, const Token[]);
EvalResult eval_expr_list(const Expression *);
EvalResult eval(const char *);

#endif // !_SCHE_LIB_H
