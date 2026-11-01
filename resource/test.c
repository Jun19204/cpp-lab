#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
  int array[10] = {1,2,3,4,5,6,7,8,9,10};
  size_t size = sizeof(array) / sizeof(array[0]);
  int sum = 0;
  for (int* iter = array; iter != &array[size]; ++iter) {
    sum += *iter;
  }
  printf("sum = %d", sum);

  return EXIT_SUCCESS;
}
