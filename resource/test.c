int main(void) {
  int array[] = {1,2,3};
  int size = sizeof(array) / sizeof(array[0]);
  int sum = 0;
  for (int* iter = array; iter != &array[size]; ++iter) {
    sum += *iter;
  }
  return 0;
}
