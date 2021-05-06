This project was made by Austin Morris and Jasmine Parsons.

It contains a shell.c, Makefile, and dieWithError.c

shell.c is missing the bash functionality (we tried), but should work otherwise. It simulates a simple shell.

Makefile creates the executable which can be run with ./shell

dieWithError.c should never be seen by the user, as it exits the program, but it was used to catch fatal errors in development, such as a returned error from fork().

Austin implemented the scanner function which separates the user input into tokens. He also implemented several of the parser functions, such as setVar(), changeDir(), and isComment(). Jasmine created the other parser functions and the shell_execute() function. She also worked vigorously on the bash functionality, however, we could not get it working in time.
