int main(void) {
  int array[10] = {1,2,3,4,5,6,7,8,9,10};
  int size = sizeof(array) / sizeof(array[0]);
  int sum = 0;
  for (int* iter = array; iter != &array[size]; ++iter) {
    sum += *iter;
  }
  return 0;
}
