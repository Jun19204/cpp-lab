#pragma once

#include "Blob/Blob.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
template <typename T> class BlobPtr {
public:
  BlobPtr(): curr_(0) {}
  BlobPtr(Blob<T>& a, size_t sz = 0)
    : wptr_(a.data), curr_(sz) {}
  T& operator*() {
    auto p = check(curr_, "dereference past end");
    return (*p)[curr_]; // (*p) is the vector to which this object points
  }
  const T& operator*() const {
    auto p = check(curr_, "dereference past end");
    return (*p)[curr_];
  }
  T* operator->() {
    return &this->operator*(); // &**this와 동일
  }
  const T* operator->() const {
    return &this->operator*(); // &**this와 동일!
                               // operator&까지 오버로딩해버린
                               // 멍청한 사용자까지 방어한다면
                               // std::addressof(**this)
  }
  BlobPtr& operator++();
  BlobPtr& operator--();
  BlobPtr operator++(int);
  BlobPtr operator--(int);
private:
  // check returns a shared_ptr to the vector if the check succeeds
  std::shared_ptr<std::vector<T>>
    check(std::size_t, const std::string&) const;
  // store a weak_ptr, which means the underlying vector might be destroyed
  std::weak_ptr<std::vector<T>> wptr_;
  std::size_t curr_; // current position within the array
};


template <typename T>
BlobPtr<T>& BlobPtr<T>::operator++() {
  ++curr_;
  return *this;
}
template <typename T>
BlobPtr<T>& BlobPtr<T>::operator--() {
  --curr_;
  return *this;
}

template <typename T>
BlobPtr<T> BlobPtr<T>::operator++(int) {
  BlobPtr ret = *this;
  ++*this;
  return ret;
}
template <typename T>
BlobPtr<T> BlobPtr<T>::operator--(int) {
  BlobPtr ret = *this;
  --*this;
  return ret;
}


