#include "sche_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    puts("__   __   _        _                _   _");
    puts("\\ \\ / /__| |_     / \\   _ __   ___ | |_| |__   ___ _ __");
    puts(" \\ V / _ \\ __|   / _ \\ | '_ \\ / _ \\| __| '_ \\ / _ \\ '__|");
    puts("  | |  __/ |_   / ___ \\| | | | (_) | |_| | | |  __/ |");
    puts("  |_|\\___|\\__| /_/   \\_\\_| |_|\\___/ \\__|_| |_|\\___|_|");
    puts("");
    puts("          _           __            __");
    puts(" ___  ___| |__   ___ / / __ ___   __\\ \\");
    puts("/ __|/ __| '_ \\ / _ \\ | '_ ` _ \\ / _ \\ |");
    puts("\\__ \\ (__| | | |  __/ | | | | | |  __/ |");
    puts("|___/\\___|_| |_|\\___| |_| |_| |_|\\___|_|");
    puts("                     \\_\\            /_/");
    puts(" ___       _                           _");
    puts("|_ _|_ __ | |_ ___ _ __ _ __  _ __ ___| |_ ___ _ __");
    puts(" | || '_ \\| __/ _ \\ '__| '_ \\| '__/ _ \\ __/ _ \\ '__|");
    puts(" | || | | | ||  __/ |  | |_) | | |  __/ ||  __/ |");
    puts("|___|_| |_|\\__\\___|_|  | .__/|_|  \\___|\\__\\___|_|");
    puts("                       |_|");
    puts("");
    puts("Usage:");
    printf("%15s    %s\n", "-c expr", "Evaluate expression (in quotes)");
    puts("");
    return 1;
  }
  if (strcmp(argv[1], "-c") == 0) {
    if (argc < 3) {
      puts("No expression provided");
      goto error_and_clean_up;
    }
    if (argc > 3) {
      puts("Too many arguments");
      goto error_and_clean_up;
    }
    EvalResult eval_result = eval(argv[2]);
    switch (eval_result.type) {
    case EVAL_SUCCESS: {
      printf("%ld\n", eval_result.value.num);
      goto success_and_clean_up;
    }
    case EVAL_ERROR_TOKENIZATION:
      puts("Error tokenizing expression");
      break;
    case EVAL_ERROR_PARSING:
      puts("Error parsing expression");
      break;
    case EVAL_ERROR_NAME_EXPECTED:
      puts("Named expression expected");
      break;
    case EVAL_ERROR_WRONG_ARGS_N:
      puts("Wrong number of arguments (you probably noticed, don't you?)");
      break;
    case EVAL_ERROR_MEMORY_ALLOC:
      puts("Memory allocation error during evaluation");
      break;
    case EVAL_ERROR_NAME_OR_SUBEXPR_EXPECTED:
      puts("Integer or subexpression expected");
      break;
    case EVAL_ERROR_UNDEFINED_FUNCTION:
      puts("Error evaluating expression");
      break;
    }
    goto error_and_clean_up;
  }
success_and_clean_up:
  return EXIT_SUCCESS;

error_and_clean_up:
  return EXIT_FAILURE;
}
