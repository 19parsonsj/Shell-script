all: shell

shell: shell.c dieWithError.c
  gcc -Og -o shell shell.c dieWithError.c
