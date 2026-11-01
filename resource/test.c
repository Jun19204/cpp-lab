int sum(int arr[], int n) {
  int total = 0;
  int* iter = arr;
  int* end = arr + n;
  for (; iter != end; ++iter) {
    total += *iter;
  }
  return total;
}

int main(void) {
  int array[] = {1, 2, 3, 4, 5};
  int size = sizeof(array) / sizeof(array[0]);
  int result = sum(array, size);
  return 0;
}
