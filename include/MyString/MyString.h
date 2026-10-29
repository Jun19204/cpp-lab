#ifndef MYSTRING_H
#define MYSTRING_H

#include <cstddef>
#include <iterator>
#include <ostream>

class MyString {
  friend void swap(MyString& lhs, MyString& rhs) noexcept;
  friend MyString operator+(const MyString& lhs, const MyString& rhs);
  friend bool operator<(const MyString& lhs, const MyString& rhs) noexcept;
  friend std::ostream& operator<<(std::ostream& os, const MyString& obj);
public:
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using value_type = char;
  using reference = char&;
  using const_reference = const char&;
  using iterator = char*;
  using const_iterator = const char*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  MyString();
  MyString(std::size_t len);
  MyString(const char* raw_str);
  MyString(const MyString& other);
  MyString(MyString&& other) noexcept;
  ~MyString();
  MyString& operator=(const MyString& other);
  MyString& operator=(MyString&& other) noexcept;
  MyString& operator+=(const MyString& other);
  char& operator[](std::size_t i);
  const char& operator[](std::size_t i) const;

  [[nodiscard]] size_type size() const noexcept;
  [[nodiscard]] char* data() noexcept;
  [[nodiscard]] const char* data() const noexcept;
  [[nodiscard]] bool empty() const noexcept;
  [[nodiscard]] iterator begin() noexcept;
  [[nodiscard]] const_iterator begin() const noexcept; 
  [[nodiscard]] iterator end() noexcept;
  [[nodiscard]] const_iterator end() const noexcept; 
  [[nodiscard]] const_iterator cbegin() const noexcept;
  [[nodiscard]] const_iterator cend() const noexcept;
  [[nodiscard]] reverse_iterator rbegin() noexcept;
  [[nodiscard]] const_reverse_iterator rbegin() const noexcept; 
  [[nodiscard]] reverse_iterator rend() noexcept;
  [[nodiscard]] const_reverse_iterator rend() const noexcept; 
  [[nodiscard]] const_reverse_iterator crbegin() const noexcept; 
  [[nodiscard]] const_reverse_iterator crend() const noexcept; 

private:
  std::size_t len_;
  char* data_;
};


bool operator>(const MyString& lhs, const MyString& rhs) noexcept;
bool operator<=(const MyString& lhs, const MyString& rhs) noexcept;
bool operator>=(const MyString& lhs, const MyString& rhs) noexcept;
bool operator==(const MyString& lhs, const MyString& rhs) noexcept;
bool operator!=(const MyString& lhs, const MyString& rhs) noexcept;

#endif
