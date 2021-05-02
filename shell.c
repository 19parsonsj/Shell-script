#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#define MAX 256
#define TOKEN_BUFSIZE 64
#define TOKEN_WHITESPACE " \t\n\r"

void dieWithError(char *);

// Fgets wrapper
char *Fgets(char *ptr, int n, FILE *stream) {
  char *rptr;
  if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
    dieWithError("Fgets error");
  return rptr;
}

char **shell_scanner(char *input) {
  int bufsize = TOKEN_BUFSIZE;
  int pos = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens)
    dieWithError("Malloc allocation error");
 
  token = strtok(input, TOKEN_WHITESPACE); 
  while (token != NULL) {
    // printf tokens for debugging
    printf("token %d: %s\n", pos, token);
    
    tokens[pos] = token;
    pos++;

    if (pos >= bufsize)
      dieWithError("too many tokens");
   
    token = strtok(NULL, TOKEN_WHITESPACE);
  }
  tokens[pos] = NULL;

  // print tokens for DEBUGGING ONLY
  for (int i = 0; i < pos; i++)
    printf("Tokens %d: %s\n", pos, tokens[i]);
  return tokens;
}


/*
char **shell_scanner(char *input) {
  char *line = input;
  int numTokens = 0;
  int bufSize = TOKEN_BUFSIZE;
  char **tokens = malloc(bufSize * sizeof(char*));
//  char *tokens[bufSize];
  for (;;) {
    while (isspace(*line))
      line++;
    if (*line == '\0')
      break;

    if (*line == '"') {
      int doubQuote = *line++;
      int i = 0;
      char token[MAX];
      while (*line == doubQuote) {
        line++;
        token[i] = *line;
       // token++;
        i++;
      }
      if (*line == '\0')
        dieWithError("single double quote in input");
      line++;
      printf("We got here\n");
      tokens[numTokens] = token;
      printf("And we got here\n");
      numTokens++;
      continue;
    }
    if (isalnum(*line)) {
      char token[MAX];
      int i = 0;
      while (isalnum(*line)) {
        token[i] = *line;
        //token++;
        line++;
        i++;
      }
      tokens[numTokens] = token;
      numTokens++;
      continue;
    }
    dieWithError("Illegeal character");
  }

  for (int i = 0; i < numTokens; i++)
    printf("Token %d: %s\n",i,tokens[i]);

  return tokens;
}
*/

void shell_loop(void){

  char input[MAX];
  char **args;
  int x;

  do {
    printf(">> ");
    Fgets(input, MAX, stdin);
    args = shell_scanner(input);
//    x = shell_execute(args);

    free(args);
  } while(x);
}

int main(int argc, char **argv) {

  shell_loop();

}
