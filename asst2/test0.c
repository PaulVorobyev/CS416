#include <stdlib.h>
#include "my_malloc.c"

int main(){
  void *p = malloc(0);
  void *ptr = malloc(-2000);
  free(p); 
  free(ptr);
  return 0;
}
