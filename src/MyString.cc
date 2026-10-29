#include "MyString/MyString.h"

#include <utility>
#include <ostream>
#include <cstddef>

MyString::MyString() : len_(0), data_(new char[1]) {}

MyString::MyString(std::size_t len) 
  : len_(len), data_(new char[len + 1]) {
  data_[len_] = '\0';
}

MyString::MyString(const char* raw_str) : len_(0) {
  for (const char* ptr = raw_str; *ptr != '\0'; ++ptr) {
    ++len_;
  }
  data_ = new char[len_ + 1];
  const char* cptr = raw_str;
  char* iter = data_;
  for (; *cptr != '\0'; ++iter, ++cptr) {
    *iter = *cptr;
  }
  *iter = '\0';
}

MyString::MyString(const MyString& other) : MyString(other.data_) {}

MyString::MyString(MyString&& other) noexcept 
  : len_(other.len_), data_(other.data_) {
  other.data_ = nullptr;
  other.len_ = 0;
}

MyString::~MyString() {
  delete [] data_;
}

MyString& 
MyString::operator=(const MyString& other) {
  if (this != &other) {
    delete [] data_;
    len_ = other.len_;
    data_ = new char[len_ + 1];
    const char* cptr = other.data_;
    char* iter = data_;
    for (; *cptr != '\0'; ++iter, ++cptr) {
      *iter = *cptr;
    }
    *iter = '\0';
  }
  return *this;
}

MyString&
MyString::operator=(MyString&& other) noexcept {
  if (this != &other) {
    delete [] data_;
    len_ = other.len_;
    data_ = other.data_;
    other.data_ = nullptr;
    other.len_ = 0;
  }
  return *this;
}

MyString&
MyString::operator+=(const MyString& other) {
  MyString temp = *this + other;
  swap(*this, temp);
  return *this;
}

char&
MyString::operator[](std::size_t i) {
  return data_[i];
}

const char&
MyString::operator[](std::size_t i) const {
  return data_[i];
}

void 
swap(MyString& lhs, MyString& rhs) noexcept {
  using std::swap;
  swap(lhs.data_, rhs.data_);
  swap(lhs.len_, rhs.len_);
}

MyString 
operator+(const MyString& lhs, const MyString& rhs) {
  MyString ret(lhs.len_ + rhs.len_);
  const char* lhs_iter = lhs.data_;
  const char* rhs_iter = rhs.data_;
  MyString::iterator iter = ret.begin();
  for (; *lhs_iter != '\0'; ++iter, ++lhs_iter) {
    *iter = *lhs_iter;
  }
  for (; *rhs_iter != '\0'; ++iter, ++rhs_iter) {
    *iter = *rhs_iter;
  }
  return ret;
}

bool 
operator<(const MyString& lhs, const MyString& rhs) noexcept {
  const char* lhs_begin = lhs.data_;
  const char* lhs_end = lhs_begin + lhs.len_;
  const char* rhs_begin = rhs.data_;
  const char* rhs_end = rhs_begin + rhs.len_;
  for (; lhs_begin != lhs_end && rhs_begin != rhs_end; ++lhs_begin, ++rhs_begin) {
    if (*lhs_begin != *rhs_begin) {
      return *lhs_begin < *rhs_begin;
    }
  }
  if (lhs.len_ != rhs.len_) {
    return lhs.len_ < rhs.len_;
  }
  return false;
}

bool
operator>(const MyString& lhs, const MyString& rhs) noexcept {
  return rhs < lhs;
}

bool
operator<=(const MyString& lhs, const MyString& rhs) noexcept {
  return !(rhs < lhs);
}

bool
operator>=(const MyString& lhs, const MyString& rhs) noexcept {
  return !(lhs < rhs);
}

bool
operator==(const MyString& lhs, const MyString& rhs) noexcept {
  return !(lhs < rhs) && !(rhs < lhs);
}

bool
operator!=(const MyString& lhs, const MyString& rhs) noexcept {
  return (lhs < rhs) || (rhs < lhs);
}

std::ostream& 
operator<<(std::ostream& os, const MyString& obj) {
  os << obj.data_;
  return os;
}


MyString::size_type 
MyString::size() const noexcept { 
  return len_;
}

char* 
MyString::data() noexcept {
  return data_;
}
const char* 
MyString::data() const noexcept {
  return data_;
}

bool 
MyString::empty() const noexcept {
  return len_ == 0;
}

MyString::iterator 
MyString::begin() noexcept {
  return data_;
}
MyString::const_iterator
MyString::begin() const noexcept {
  return data_;
}
MyString::iterator 
MyString::end() noexcept {
  return data_ + len_;
}
MyString::const_iterator
MyString::end() const noexcept {
  return data_ + len_;
}

MyString::const_iterator 
MyString::cbegin() const noexcept {
  return data_;
}
MyString::const_iterator 
MyString::cend() const noexcept {
  return data_ + len_;
}

MyString::reverse_iterator 
MyString::rbegin() noexcept {
  return reverse_iterator(end());
}
MyString::const_reverse_iterator
MyString::rbegin() const noexcept {
  return const_reverse_iterator(cend());
}
MyString::reverse_iterator 
MyString::rend() noexcept {
  return reverse_iterator(begin());
}
MyString::const_reverse_iterator
MyString::rend() const noexcept {
  return const_reverse_iterator(cbegin());
}

MyString::const_reverse_iterator
MyString::crbegin() const noexcept {
  return const_reverse_iterator(cend());
}
MyString::const_reverse_iterator
MyString::crend() const noexcept {
  return const_reverse_iterator(cbegin());
}
