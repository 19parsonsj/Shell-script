#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>

#define MAX 256
#define MAX_VARS 64
#define TOKEN_BUFSIZE 64
#define TOKEN_WHITESPACE " \t\n\r"

void dieWithError(char *);

// Structure for the variables
struct variable {
 char *name;
 char *value;
}

// Global array for variables
variable varArray[MAX_VARS];

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

int changeDir(char **args) {
  if (args[1] == NULL)
    printf("Error: Expected an argument for cd\n");
  else {
    if (chdir(args[1]) != 0)
      printf("Error: Invalid directory\n");
    // Update the value of CWD
    if (getcwd(CWD, sizeof(CWD) == NULL)
        printf("Error: getcwd failure\n");
  }
  return 1;
}

int isComment(char **args) {
  char *p;
  int foundHash = 0;
  //Search the strings for a #
  for (int i = 0; i < sizeof(args); i++)
    if (p = strchr(args[i],'#') != NULL)
      foundHash = 1;
  
  // If a "#" wasn't found, return 0. If it was found, it should be the very first char
  // in the first token. If it isn't, it's a syntax error
  if (foundHash == 0)
    return 0;
  else if (args[0][0] == '#')
    return 1;
  else {
   printf("Error: Unexpected character \"#\"\n");
   return -1;
  }
}

// Test if any tokens are labeled as variables. If they are, repalce that token with
// their associated variable values. If the variable is not declared, syntax error.
int fetchVar(char **args) {
  char *p;
  int foundSign = 0;
  //Search the tokens for any $. If it is found, it should be the very first char
  // in any given token. The next char should be a letter according to variable assignment
  // rules. If it isn't, it's a syntax error.
  for (int i = 0; i < sizeof(args); i++)
    if (p = strchr(args[i],'$') != NULL) {
      if (args[i][0] != '$') {
        printf("Error: Unexpected use of '$' character.\n");
        return -1;
      }
      // Else if the second character is NOT a letter
      else if ( !(isalpha(args[i][1])) ) {
        printf("Error: Invalid use of '$'");
        return -1;
      }
    }
  
  // If a "$" wasn't found, return 0. 
  if (foundSign == 0)
    return 0;
  
  
  
  else if (args[0][0] == '#')
    return 1;
  else {
   printf("Error: Unexpected character \"#\"\n");
   return 1;
}

// If the first token is alphanumerical with its first character being a letter, AND the second
// token is "=", then set variable. Note that if "variable = value" has no spaces, (e.g.
// "variable=value") then it is a syntax error for an unknown command.
int setVar(char **args) {
  // Test if there are 3 tokens. If not, it is an invalid variable assignment.
  if (sizeof(args) != 3) {
    printf("Error: Invlaid variable assignment. Expected \"Variable = value\"\n"); 
    return 1;
  }
  
}

int listVar(char **args) {
  
}

// Remeber it is an error to unset a built in variable
int unsetVar(char **args) {
  
}


//parser

//pass results of scanner to parser
//don't forget print out error messages 


//function to build list of tokens 
//when scanner returns end of line indication: use list to analyze command



//is it a command?
  //#

  //cd

  //variable

  //lv

  //unset variable
  
  // !f
    //fork 
      //after forking, parent should wait for command it has just 
      //started to complete before issuing a promt for next command

      //if includes: infrom or outto
        //the child processs hould first attempt to open or create the named file(s)
        //if successful
          //close the indicated file descriptor (standard input or standard output: 0 or 1)
          //and re-associate the descriptor number(s) with the newly opened file(s). (look at man for dup2) 
          //then can execve

    //execve new process to execute given file

  //quit


//built in variables?

  //PATH

  //CWD

  //PS


//Name of program to run?


//is a parameter of command?


void shell_loop(void){

  char *input = (char*)malloc(MAX);
  char **args;
  int x;

  do {
    printf(">> ");
    Fgets(input, MAX, stdin);
    args = shell_scanner(input);
    x = shell_execute(args);

    free(input)
    free(args);
  } while(x);
}

int main(int argc, char **argv) {

  shell_loop();

}
