#pragma once

#include <cstring>
#include <functional>

template <typename T>
int compare(const T& left, const T& right) {
  std::less<T> less; 
  if (less(left, right)) {
    return -1;
  }
  if (less(right, left)) {
    return 1;
  }
  return 0;
}


template <unsigned N, unsigned M>
int compare(const char (&left) [N], const char (&right) [M]) {
  return strcmp(left, right);
}
