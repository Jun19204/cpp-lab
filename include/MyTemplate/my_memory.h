#pragma once

#include <cstddef>
#include <utility>
#include <functional>

template <typename T>
class my_shared_ptr {
public:
  using value_type = T;
  using size_type = std::size_t;
  using deleter_type = std::function<void(T*)>;

  friend void swap(my_shared_ptr& lhs, my_shared_ptr& rhs) noexcept {
    lhs.swap(rhs);
  };

  explicit my_shared_ptr(T* raw_ptr, deleter_type del=nullptr);
  my_shared_ptr(const my_shared_ptr& other);
  my_shared_ptr(my_shared_ptr&& other) noexcept;
  
  my_shared_ptr& operator=(const my_shared_ptr& other);
  my_shared_ptr& operator=(my_shared_ptr&& other) noexcept;
  
  ~my_shared_ptr();

  [[nodiscard]] T* get() const noexcept {
    return ptr_;
  }

  [[nodiscard]] size_type use_count() const noexcept {
    return ref_cnt_ != nullptr ? *ref_cnt_ : 0;
  }

  [[nodiscard]] T& operator*() const noexcept {
    return *ptr_;
  }

  [[nodiscard]] T* operator->() const noexcept {
    return &**this;
  }

  explicit operator bool() const noexcept {
    return ptr_ != nullptr;
  }

  void swap(my_shared_ptr& rhs) noexcept {
    using std::swap;
    swap(ptr_, rhs.ptr_);
    swap(ref_cnt_, rhs.ref_cnt_);
    swap(del_, rhs.del_);
  }


private:
  void add_ref();
  void release();
  T* ptr_{nullptr};
  size_type* ref_cnt_{nullptr};
  deleter_type del_{nullptr};
};



template <typename T>
void my_shared_ptr<T>::add_ref() {
  if (ref_cnt_ != nullptr) {
    ++*ref_cnt_;
  }
}

template <typename T>
void my_shared_ptr<T>::release() {
  if (ref_cnt_ == nullptr) {
    return;
  }
  --*ref_cnt_;
  if (*ref_cnt_ == 0) {
    (del_ != nullptr) ? del_(ptr_) : delete ptr_;
    delete ref_cnt_;
  }
  ptr_ = nullptr;
  ref_cnt_ = nullptr;
  del_ = nullptr;
}

// 생성자
template <typename T>
my_shared_ptr<T>::my_shared_ptr(T* raw_ptr, deleter_type del) 
  : ptr_{raw_ptr}, 
    ref_cnt_{new std::size_t(1)}, 
    del_{del} { }

// 복사 생성자
template <typename T>
my_shared_ptr<T>::my_shared_ptr(const my_shared_ptr& other)
  : ptr_{other.ptr_}, 
    ref_cnt_{other.ref_cnt_}, 
    del_{other.del_} { 
  add_ref();
}

// 이동 생성자
template <typename T>
my_shared_ptr<T>::my_shared_ptr(my_shared_ptr&& other) noexcept
  : ptr_{other.ptr_},
    ref_cnt_{other.ref_cnt_}, 
    del_{std::move(other.del_)} {
  other.ptr_ = nullptr;
  other.ref_cnt_ = nullptr;
  other.del_ = nullptr;
}

// 복사 대입 연산자
template <typename T>
my_shared_ptr<T>& 
my_shared_ptr<T>::operator=(const my_shared_ptr& other) {
  if (this == &other) {
    return *this;
  }
  release();
  ptr_ = other.ptr_;
  ref_cnt_ = other.ref_cnt_;
  del_ = other.del_;
  add_ref();
  return *this;
}

// 이동 대입 연산자
template <typename T>
my_shared_ptr<T>&
my_shared_ptr<T>::operator=(my_shared_ptr&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  release();
  ptr_ = other.ptr_;
  ref_cnt_ = other.ref_cnt_;
  del_ = std::move(other.del_);
  other.ptr_ = nullptr;
  other.ref_cnt_ = nullptr;
  return *this;
}

// 소멸자
template <typename T>
my_shared_ptr<T>::~my_shared_ptr() {
  release();
}






template <typename T, typename D>
class my_unique_ptr {
  friend void swap(T* lhs, T* rhs);
public:
  using deleter_type = std::function<D>;

  [[nodiscard]] T* get();

private:
  T* ptr_{nullptr};
  deleter_type del_{nullptr};
};







