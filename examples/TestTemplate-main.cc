#include "MyTemplate/myfind.h"

#include <cstdlib>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

int main() {
  std::vector<std::string> str_vec;
  using size_type = std::vector<std::string>::size_type;

  for (size_type i = 0; i < 5; ++i) {
    std::cout << "word: ";
    std::string element;
    std::cin >> element;
    str_vec.emplace_back(std::move(element));
  }

  std::cout << "\nword to find: ";
  std::string input;
  while (std::cin >> input && input != "q") {
    if (myfind(str_vec.begin(), str_vec.end(), input) != str_vec.end()) {
      std::cout << "찾았습니다!\n";
      break;
    }
    std::cout << "못찾았습니다...\n";
    std::cout << "\nword to find: ";
  }

  return EXIT_SUCCESS;
} 
