#include "sche_lib.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
  {
    const size_t test_size = 2048;

    char many_tokens[test_size];
    memset(many_tokens, '(', test_size - 1);
    many_tokens[test_size - 1] = '\0';

    TokenizerResult tok_result = tokenize(many_tokens);
    assert(tok_result.status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 2047);
  }
  {
    TokenizerResult tok_result = tokenize("(+ -1 20)");
    TokenizerStatus tok_status = tok_result.status;
    assert(tok_status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 5);

    assert(tok_result.token_list.tokens[0].type == TOKEN_TYPE_PAR_OPEN);

    assert(tok_result.token_list.tokens[1].type == TOKEN_TYPE_NAME);
    assert(*tok_result.token_list.tokens[1].value.s == '+');

    assert(tok_result.token_list.tokens[2].type == TOKEN_TYPE_NUMBER);
    assert(tok_result.token_list.tokens[2].value.num == -1);

    assert(tok_result.token_list.tokens[3].type == TOKEN_TYPE_NUMBER);
    assert(tok_result.token_list.tokens[3].value.num == 20);

    assert(tok_result.token_list.tokens[4].type == TOKEN_TYPE_PAR_CLOSE);
  }
  {
    TokenizerResult tok_result = tokenize("99c");
    assert(tok_result.status == TOKENIZER_ERROR_INVALID_NAME);
  }
  {
    TokenizerResult tok_result = tokenize("e2e4 abc");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 2);

    assert(tok_result.token_list.tokens[0].type == TOKEN_TYPE_NAME);
    assert(strncmp(tok_result.token_list.tokens[0].value.s, "e2e4", 4) == 0);

    assert(tok_result.token_list.tokens[1].type == TOKEN_TYPE_NAME);
    assert(strncmp(tok_result.token_list.tokens[1].value.s, "abc", 3) == 0);
  }
  {
    assert(tokenize("").status == TOKENIZER_SUCCESS);
    assert(tokenize("my-variable").status == TOKENIZER_SUCCESS);
    assert(tokenize("*special*").status == TOKENIZER_SUCCESS);
    assert(tokenize("+special+").status == TOKENIZER_SUCCESS);
    assert(tokenize("/divide/").status == TOKENIZER_SUCCESS);
    assert(tokenize("=eq=").status == TOKENIZER_SUCCESS);
    assert(tokenize("variable_with_underscore").status == TOKENIZER_SUCCESS);
  }
  {
    TokenizerResult tok_result = tokenize("");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 0);
    assert(parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens)
               .type == PARSE_ERROR_TOO_SHORT_EXPR);
  }
  {
    TokenizerResult tok_result = tokenize("()");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    assert(tok_result.token_list.tokens_n == 2);
    assert(parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens)
               .type == PARSE_ERROR_TOO_SHORT_EXPR);
  }
  {
    TokenizerResult tok_result = tokenize("(+ 1 2)");
    TokenizerStatus tok_status = tok_result.status;
    assert(tok_status == TOKENIZER_SUCCESS);

    ParserResult pr =
        parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens);
    assert(pr.type == PARSE_SUCCESS);
    assert(pr.expr->type == EXPR_TYPE_SUBEXPR);
    assert(strncmp(pr.expr->value.s, "+", 1) == 0);

    assert(pr.expr->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->value.num == 1);

    assert(pr.expr->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->value.num == 2);
    assert(pr.expr->next->next->next == NULL);

    EvalResult er = eval_expr_list(pr.expr);
    assert(er.type == EVAL_SUCCESS);
    assert(er.value.num == 3);
  }
  {
    EvalResult er = eval("(- 0777 0555)");
    assert(er.type == EVAL_SUCCESS);
    assert(er.value.num == 146);
  }
  {
    EvalResult er = eval("(+ 0xdead0000 0xbeef)");
    assert(er.type == EVAL_SUCCESS);
    assert(er.value.num == 0xdeadbeef);
  }
  {
    TokenizerResult tok_result = tokenize("(+ (- 5 4) 40 1)");
    assert(tok_result.status == TOKENIZER_SUCCESS);
    ParserResult pr =
        parse(tok_result.token_list.tokens_n, tok_result.token_list.tokens);
    assert(pr.type == PARSE_SUCCESS);
    assert(pr.expr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->value.s == '+');

    assert(pr.expr->next != NULL);
    assert(pr.expr->next->type == EXPR_TYPE_SUBEXPR);
    assert(pr.expr->next->subexpr != NULL);
    assert(pr.expr->next->subexpr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->next->subexpr->value.s == '-');
    assert(pr.expr->next->subexpr->next->value.num == 5);
    assert(pr.expr->next->subexpr->next->next->value.num == 4);
    assert(pr.expr->next->subexpr->next->next->next == NULL);

    assert(pr.expr->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->value.num == 40);
    assert(pr.expr->next->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->next->value.num == 1);
    assert(pr.expr->next->next->next->next == NULL);

    EvalResult er = eval_expr_list(pr.expr);
    assert(er.type == EVAL_SUCCESS);
    assert(er.value.num == 42);

    // and nothing changed after evaluation
    assert(pr.expr->next->type == EXPR_TYPE_SUBEXPR);
    assert(pr.expr->next->subexpr != NULL);
    assert(pr.expr->next->subexpr->type == EXPR_TYPE_SUBEXPR);
    assert(*pr.expr->next->subexpr->value.s == '-');
    assert(pr.expr->next->subexpr->next->value.num == 5);
    assert(pr.expr->next->subexpr->next->next->value.num == 4);

    assert(pr.expr->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->value.num == 40);
    assert(pr.expr->next->next->next->type == EXPR_TYPE_INT);
    assert(pr.expr->next->next->next->value.num == 1);
  }
  printf("\x1b[32m"); // green text
  printf("\u2713 ");  // Unicode check mark
  printf("\x1b[0m");  // Reset text color to default
  printf("All tests passed\n");
}
