#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <libgen.h>

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
  numTokens = 0;
  if (!tokens)
    dieWithError("Malloc allocation error");
 
  token = strtok(input, TOKEN_WHITESPACE); 
  while (token != NULL) {
    // printf tokens for debugging
    printf("token %d: %s\n", numTokens, token);
    
    tokens[numTokens] = token;
    numTokens++;

    if (numTokens >= bufsize)
      dieWithError("too many tokens");
   
    token = strtok(NULL, TOKEN_WHITESPACE);
  }
  tokens[numTokens] = NULL;
  printf("zero value: %s\n", tokens[0]);
  // print tokens for DEBUGGING ONLY
  for (int i = 0; i < numTokens; i++)
    printf("Tokens %d: %s\n", numTokens, tokens[i]);
  return tokens;
}

// Changes the current working directory
int changeDir(char **args) {
  printf("in changedir\n");
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
  printf("we are in isComment\n");
  printf("zero value: %s\n", args[0]);
  char *p;
  int foundHash = 0;
  //Search the strings for a #
  for (int i = 0; i < numTokens; i++)
  {
    //if ((p == strchr(args[i],'#')) != NULL)
    p = strchr(args[i],'#');
    if (p[0] != '\0')
      printf("found hash\n");
      foundHash = 1;
  }
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
      if (point == strchr(args[j],'$') != NULL) {
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
          args[j] = newToken;
          args[j] = newToken;
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
  printf("we are in setVar");
  if (isalpha(*args[0]) == 0) {
    printf("Error: Invalid variable assignment. First character must be numerical.\n");
    return 1;
  }
  else if (strcmp(args[0], "CWD") == 0) {
    printf("Error: Cannot change variable \"CWD\"\n");
    return 1;
  }
  while (*args[0] != '\0') {
    if (isalnum(*args[0]) == 0) {
      printf("Error: Invalid variable assignment. Variable can only be composed of alpha-numerical characters.\n");
      return 1;
    }
    args[0]++;
  }

  // Append a '$' to the beginning of the new variable name to simplify searching for variables.
  char *ch = "$";
  char *newName = strcat(ch, args[0]);

   // Make sure this variable has not been declared before. If it has overwrite the value. If it hasn't create a new one.
  int isNewVar = 0;
  int overwriteVal = 0;
 
  for (int i = 0; i < countVar; i++) {
    if (varArray[i].name == newName) {
      isNewVar = 1;
      overwriteVal = i;
    }
  }
   // There is no need to overwrite the name if the variable has been declared before. Simply overwrite the value.
  if (isNewVar == 1) {
    varArray[overwriteVal].value = args[2];
    return 1;
  }
 else {
  varArray[countVar].name = newName; 
  varArray[countVar].value = args[2];
  countVar++;
  return 1;
 }
}

int listVar(char **args) {
  printf("In listVar\n");
  printf("Variable List: \n");
  for (int i = 0; i < countVar; i++)
    printf("Name: %s\t Value: %s\n", varArray[i].name, varArray[i].value);
  return 1;
}


// Remeber it is an error to unset a built in variable
int unsetVar(char **args) {
  if (args[1] == varArray[0].name || args[1] == varArray[1].name || args[1] == varArray[2].name)
      printf("Error: cannot unset defined variables.");
  int varFound = 0;
  // Find the variable desired to be removed
    for (int i = 0; i < countVar-1; i++){
      if (varArray[i].name == args[1]) {
        // Once found, replace elements in the array to simulate the element being removed
        varFound = 1;
        for (int j = i; j < countVar; j++) {
          varArray[j] = varArray[j+1];
        }
      }
    }
  if (varFound == 1) {
    // The element has been found and deleted, now decrement count to dispose of the excess variable.
    countVar--;
    return 1;
  }
  else { // Else the element wasn't declared, which is an error.
    printf("Error: Could not find the desired variable to remove.");
    return 1;
  }
}


//execute program function

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

  //EXECUTE PROGRAM FUNCTION
  int execute_program(char **args) {
  printf("in !\n");
  pid_t pid, wpid;
	int fd0, fd1; 

  //fork
	if((pid=fork())==-1){
		printf("Child process could not be created\n");
		return 1;
	}
  //child
	if(pid==0){
		// check to see if infrom or outto could be tokens
		if (sizeof(args) > 2){
      //check to see if there is an infrom token
      if ((strcmp(args[2], "infrom:") == 0) || ((strcmp(args[4], "infrom:")) == 0)){
			// We open (create) the file from infrom: _____
			fd0 = open(args[3]); 
			dup2(fd0, STDOUT_FILENO); 
			close(fd0);
      //re associate descriptor numbers
     
      }
      //check if there is outto by itself or with infrom 
      if (args[4] == "outto:") == 0) || args[2] == "outto:") {
        //open
        fd1 = open(args[5], 0644);
        dup2(fd1, STDOUT_FILENO);
        //close
        close(fd1);
        //re associate descriptor numbers
       

      }
    
    execvp(args[1], args);
		
		}
  }
    //parent
    else {
      //wait for child to finish before prompting for next command
      wpid = waitpid(pid,NULL,0);

		if (execvp(args[1],args) == 0){
			printf("Error: could not recognise use of !");
			kill(getpid(),SIGINT);
		}	
    return 1;	 
	}

}


//works
int exit_shell() {
  //exit with status zero
  printf("Exiting shell: exit status zero.\n");
  return 0;
}



int shell_execute(char **args) {
printf("we in execute\n");

int i = isComment(args);

  if (i == 0) {
    printf("out of iscomment\n");
  
  if (numTokens == 0){
    printf("No arguments found!\n");
    return 1;
  }
  if (args[0][0] == '#') 
    return isComment(args);
  
  else if (strcmp(args[0], "cd") == 0) 
    return changeDir(args);
   
  else if (strcmp(args[0],"lv") == 0) 
    return listVar(args);
  
  else if (args[0][0] == '!') 
    return execute_program(args);
    
  else if (strcmp(args[0], "unset") == 0)
    return unsetVar(args);
 
  else if (numTokens == 3 && *args[1] == '='){
    printf("going to setvar\n");
    return setVar(args);
  }
  else if ((strcmp(args[0], "quit")) == 0)
    return exit_shell();

}
else {
  printf("returning 1\n");
  return 1;
}

}

void shell_loop(void){

  char *input = (char*)malloc(MAX);
  char **args;
  int x;

  do {
    printf("%s",varArray[2].value);
    //added input = 
    Fgets(input, MAX, stdin);
    if (*input == '\0')
      x = exit_shell();
    else {
    args = shell_scanner(input);
    printf("calling execute\n");
    x = shell_execute(args);
    printf("after execute\n");
    }
    //if (input != NULL && args != NULL)
    //free(input);

    //free(args);
    
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
