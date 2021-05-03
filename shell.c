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
};

// Global array for variables. varArray[0] = PATH, varArray[1] = CWD,
// PS = varArray[2]
struct variable varArray[MAX_VARS];
int countVar = 3;

int numTokens = 0;

// Fgets wrapper
char *Fgets(char *ptr, int n, FILE *stream) {
  char *rptr;
  if (((rptr = fgets(ptr, n, stream)) == NULL) && ferror(stream))
    dieWithError("Fgets error");
  return rptr;
}

// Takes the user input and makes them into seperate tokens
char **shell_scanner(char *input) {
  int bufsize = TOKEN_BUFSIZE;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens)
    dieWithError("Malloc allocation error");
 
  token = strtok(input, TOKEN_WHITESPACE); 
  while (token != NULL) {
    // printf tokens for debugging
    printf("token %d: %s\n", numTokens, token);
    
    tokens[pos] = token;
    numTokens++;

    if (numTokens >= bufsize)
      dieWithError("too many tokens");
   
    token = strtok(NULL, TOKEN_WHITESPACE);
  }
  tokens[numTokens] = NULL;

  // print tokens for DEBUGGING ONLY
  for (int i = 0; i < numTokens; i++)
    printf("Tokens %d: %s\n", numTokens, tokens[i]);
  return tokens;
}

// Changes the current working directory
int changeDir(char **args) {
  if (args[1] == NULL)
    printf("Error: Expected an argument for cd\n");
  else if (numTokens != 2)
    printf("Error: Invalid number of arguments.\n");
  else {
    if (chdir(args[1]) != 0)
      printf("Error: Invalid directory\n");
    // Update the value of CWD
    if (getcwd(varArray[1].value, sizeof(varArray[1].value) == NULL))
        printf("Error: getcwd failure\n");
  }
  return 1;
}

// Tests if there are any '#' characters. If there are, it tests if it is used
// properly or if it is a syntax error.
int isComment(char **args) {
  char *p;
  int foundHash = 0;
  //Search the strings for a #
  for (int i = 0; i < numTokens; i++)
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
   return 1;
  }
}

// Test if any tokens are labeled as variables. If they are, repalce that token with
// their associated variable values. If the variable is not declared, syntax error.
int fetchVar(char **args) {
  
  int foundSigns = 0;
  //Search the tokens for any $. If it is found, the next char should be a letter
  // according to variable assignment rules. If it isn't, it's a syntax error.
  for (int j = 0; j < numTokens; j++) {
    char *point = "p";
    int doAgain = 0;
    char newToken[MAX];
    char newTokenPiece[MAX];
    int numVars = 0;
    int position[MAX];
    // If strchr() finds a '$' in a token, go into a while loop that is able to detect multiple
    // variables in a token. It also catches any error and replaces the variables names with
    // their values in **args
    while (doAgain == 1) {
      if (point = strchr(args[j],'$') != NULL) {
        foundSigns++;
        char varName[MAX];
        int i = 0;
        char *ptr = point;
        // Get the variable name
        // First char will be '$', so do the first pattern outside the loop.
        varName[i] = *point;
        i++;
        point++;
        // the variable is made up of numbers and letters, so just look for the first non
        // alpha-numerical character after the initial '$'
        while(isalnum(*point)) {
          varName[i] = *point;
          i++;
          point++;
        }
        varName[i] = '\0';
        
        // If i==1, then that means '$' was used with non alpha-numeric characters immediately after
        // it. According to our variable rules, the first character of a variable is a letter, thus,
        // this would imply an incorrect use of the '$' symbol, regardless of what the user is doing.
        if (i == 1) {
          printf("Error: Incorrect usage of the '$' character.");
        }
     
        // Create a token piece which contains any characters before the variable in the
        // original token
        position[numVars] = ptr - args[j];
        for (int k = 0; k < position[numVars]; k++) {
          newTokenPiece[k] = *point;
          args[j]++;
        }
       
        strcat(newToken, newTokenPiece);
        // Now we have the variable's name in varName. Search for the associated variable's value
        // in varArray. If it doesn't exist, it's an error for undefined variable.
        int foundMatches = 0;
        for (int k = 0; k < countVar; k++) {
          if (strcmp(varArray[k].name, varName) == 0) {
            strcat(newToken, varArray[k].value);
            foundMatches++;
          }
        }
        
        // At this point, we know there is at leas one '$' character in the token. However,
        // if we don't get any matches in our variable array, it means the variable is
        // undefined, resulting in an error.
        if (foundMatches == 0) {
          printf("Error: Undefined variable.");
          return 1;
        }
       
        // If we are not at the NULL byte of the token, there is still more in the token. We should
        // thus check for more variables in this one token. If the NULL byte is there, then the new
        // token should replace the old one in **args.
        if (*point == '\0') {
          doAgain = 0;
          args[j] = newToken
        }
        else
          args[j] = point;
      }
      // If there is no '$' character detected, exit the while loop and move to the next token
      else
        doAgain = 0;
    }
  }
  // We're finally out of the for loop, and can now return. The while loop detects if the user
  // incorrectly used a '$' and if the variable name is not found, so there is no need to check for
  // any errors here. We can simply return normally.
  return 1;
}

// If the first token is alphanumerical with its first character being a letter, AND the second
// token is "=", then set variable. Note that if "variable = value" has no spaces, (e.g.
// "variable=value") then it is a syntax error for an unknown command.
// The only built-in variable that explictly cannot be changed according to the assignment, is CWD.
// Note that shell_execute tests if there are 3 arguments and the second argument is '=' as this
// would imply it is a setVar command. Otherwise, it will be labeled as an unknown command
int setVar(char **args) {
  if (isalpha(*args[0]) == 0) {
    printf("Error: Invalid variable assignment. First character must be numerical.\n");
    return 1;
  }
  else if (strcmp(args[0], "CWD") == 0) {
    printf("Error: Cannot change variable \"CWD\"\n");
    returns 1;
  }
  while (*args[0] != '\0') {
    if (isalnum(*args[0]) == 0) {
      printf("Error: Invalid variable assignment. Variable can only be composed of alpha-numerical characters.\n");
      return 1;
    }
    *args[0]++;
  }

  // Append a '$' to the beginning of the new variable name to simplify searching for variables.
  char *ch = "$"
  char *newName = strcat(ch, args[0]);
 
  varArray[count].name = newName; 
  varArray[count].value = args[2];
  count++;
  return 1;
}

int listVar(char **args) {
  printf("Variable List: \n");
  for (int i = 0; i < count; i++)
    printf("Name: %p\t Value: %p\n", varArray[i].name, varArray[i].value);
  return 1;
}

// Remeber it is an error to unset a built in variable
int unsetVar(char **args) {
  if (args[1] == varArray[0].name || args[1] == varArray[1].name || args[1] == varArray[2].name)
      printf("Error: cannot unset defined variables.");
  
  else 
    for (int i = 0; i < count; i++){
      if (varArray[i].name == args[1])
        varArray[i].value == NULL;
    }
  return 1;
}

//execute program function
int execute_program(char **args) {
 // !
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

}

int exit(char **args) {
  //exit with status zero
  printf("Exiting shell: exit status zero.");
  return 0;
}



int shell_execute(char **args) {


  if (isComment(args) == 1) 
    return isComment(args);
  
  else if (args[0] == 'cd') 
    return changeDir(args);
   
  else if (args[0] == 'lv') 
    return listVar(args);
  
  else if (args[0] == '!') 
    return execute_program(args);
    
  else if (args[0] == 'unset'){
    return unsetVar(args);
  }
 
  else if (numTokens == 3 && args[1] == '=')
    return setVar(args);

  else if (args[0] == 'quit' || args[0] == '^D')
    return exit(args);

  else
   setVar(args);
 


}


void shell_loop(void){

  char *input = (char*)malloc(MAX);
  char **args;
  int x;

  do {
    printf("%s",varArray[2].value);
    Fgets(input, MAX, stdin);
    args = shell_scanner(input);
    x = shell_execute(args);

    free(input);
    free(args);
  } while(x);
  
}

int main(int argc, char **argv) {
  // Create built-in variables
  varArray[0].name = "PATH";
  varArray[0].value = "/bin:/usr/bin";
  varArray[1].name = "CWD";
  getcwd(varArray[1].value, sizeof(varArray[1].value));
  varArray[2].name = "PS";
  varArray[2].value = ">> ";
  
  
 
  shell_loop();

}
