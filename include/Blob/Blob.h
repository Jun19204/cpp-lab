#pragma once

#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <vector>
#include <utility>

template <typename T> class Blob {
public:
  using value_type = T;
  using size_type = typename std::vector<T>::size_type;
  // constructors
  Blob();
  Blob(std::initializer_list<T> il);
  // number of element in Blob
  size_type size() const { return data_->size(); }
  [[nodiscard]]
  bool empty() const { return data_->empty(); }
  // add and remove element
  void push_back(const T& t) { data_->push_back(t); }
  void push_back(T&& t) { data_->push_back(std::move(t)); }
  void pop_back() { data_->pop_back(); }
  // access to element
  T& front();
  const T& front() const;
  T& back();
  const T& back() const;
  T& operator[] (size_type i) { return data_[i]; }
  const T& operator[] (size_type i) const { return data_[i]; };
private:
  std::shared_ptr<std::vector<T>> data_;
  // throws msg if data[i] isn't valid
  void check(size_type i, const std::string& msg) const;
};

template <typename T>
Blob<T>::Blob() : data_(std::make_shared<std::vector<T>>()) {}

template <typename T>
Blob<T>::Blob(std::initializer_list<T> il) 
: data_(std::make_shared<std::vector<T>>(il)) {}

template <typename T>
void Blob<T>::check(size_type i, const std::string& msg) const {
  if (i >= data_->size()) {
    throw std::out_of_range(msg);
  }
}

template <typename T>
T& Blob<T>::front() {
  check(0, "front on empty Blob");
  return data_->front();
}
template <typename T>
const T& Blob<T>::front() const {
  check(0, "front on empty Blob");
  return data_->front();
}

template <typename T>
T& Blob<T>::back() {
  check(0, "back on empty Blob");
  return data_->back();
}
template <typename T>
const T& Blob<T>::back() const {
  check(0, "back on empty Blob");
  return data_->back();
}

