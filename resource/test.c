int sum(int arr[], int n) {
  int sum = 0;
  int* begin = arr;
  int* end = arr + n;
  for (; begin != end; ++begin) {
    sum += *begin;
  }
  return sum;
}

int main(void) {
  int array[] = {1, 2, 3, 4, 5};
  int size = sizeof(array) / sizeof(array[0]);
  int result = sum(array, size);
  return 0;
}
