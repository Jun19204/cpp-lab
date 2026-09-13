#pragma once

#include <initializer_list>
#include <memory>
#include <vector>
#include <utility>

template <typename T> class Blob {
public:
  using value_type = T;
  using size_type = typename std::vector<T>::size_type;
  // Constructor
  Blob();
  Blob(std::initializer_list<T> il);
  // Counts of element in Blob
  size_type size() const { return data_->size(); }
  [[nodiscard]]
  bool empty() const { return data_->empty(); }
  // Insert and delete element
  void push_back(const T& t) { data_->push_back(t); }
  void push_back(T&& t) { data_->push_back(std::move(t)); }
  void pop_back() { data_->pop_back(); }
  // Access to element
  T& back();
  const T& back() const;
  T& operator[] (size_type i);
  const T& operator[] (size_type i) const;
private:
  std::shared_ptr<std::vector<T>> data_;
  void check(size_type i, const std::string& msg) const;
};
