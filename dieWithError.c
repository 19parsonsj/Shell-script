#include <stdio.h>
#include <stdlib.h>

void dieWithError(char *error) {
  printf("Died with error: %s, exiting\n", error);
  exit(2);
}
