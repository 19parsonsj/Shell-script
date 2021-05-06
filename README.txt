This project was made by Austin Morris and Jasmine Parsons.

It contains a shell.c, Makefile, and dieWithError.c

shell.c is missing the bash functionality (we tried), but should work otherwise. It simulates a simple shell.

Makefile creates the executable which can be run with ./shell

dieWithError.c should never be seen by the user, as it exits the program, but it was used to catch fatal errors in development, such as a returned error from fork().
