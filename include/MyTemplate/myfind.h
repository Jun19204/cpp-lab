#pragma once

template <typename InputIt, typename T>
InputIt myfind(InputIt begin, InputIt end, const T& value) {
  while (begin != end && *begin != value) {
    ++begin;
  }
  return begin;
}
