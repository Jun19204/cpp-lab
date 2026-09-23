#pragma once

#include <cstddef>
#include <utility>
#include <functional>

// 유사 shared_ptr 클래스
template <typename T>
class my_shared_ptr {
public:
  using value_type = T;
  using size_type = std::size_t;
  using deleter_type = std::function<void(T*)>;

  friend void swap(my_shared_ptr& lhs, my_shared_ptr& rhs) noexcept {
    lhs.swap(rhs);
  };

  my_shared_ptr() noexcept = default;
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
  void release();
  void add_ref();
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

/* 생성자(메개변수 1/2개 버전) */
template <typename T>
my_shared_ptr<T>::my_shared_ptr(T* raw_ptr, deleter_type del) 
  : ptr_(raw_ptr),
    ref_cnt_(raw_ptr ? new std::size_t(1) : nullptr),
    del_(del) { }

/* 복사 생성자 */
template <typename T>
my_shared_ptr<T>::my_shared_ptr(const my_shared_ptr& other)
  : ptr_(other.ptr_), 
    ref_cnt_(other.ref_cnt_), 
    del_(other.del_) { 
  add_ref();
}

/* 이동 생성자 */
template <typename T>
my_shared_ptr<T>::my_shared_ptr(my_shared_ptr&& other) noexcept
  : ptr_(other.ptr_),
    ref_cnt_(other.ref_cnt_), 
    del_(std::move(other.del_)) {
  other.ptr_ = nullptr;
  other.ref_cnt_ = nullptr;
  other.del_ = nullptr;
}

/* 복사 대입 연산자 */
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

/* 이동 대입 연산자 */
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

/* 소멸자 */
template <typename T>
my_shared_ptr<T>::~my_shared_ptr() {
  release();
}





// 유사 unique_ptr 클래스
template <typename T, typename D=std::function<void(T*)>>
class my_unique_ptr {
public:
  using value_type = T;
  using deleter_type = D;

  friend void swap(my_unique_ptr& lhs, my_unique_ptr& rhs) noexcept {
    lhs.swap(rhs);
  }

  my_unique_ptr()
    : ptr_(nullptr), del_([](T* ptr){ delete ptr; }) { }

  explicit my_unique_ptr(T* raw_ptr)
    : ptr_(raw_ptr), del_([](T* ptr){ delete ptr; }) { }

  my_unique_ptr(T* raw_ptr, D del)
    : ptr_(raw_ptr), del_(del) { }

  my_unique_ptr(const my_unique_ptr&) = delete;

  my_unique_ptr(my_unique_ptr&& other) noexcept 
    : ptr_(nullptr), del_([](T* ptr){ delete ptr; }) { // 이동 대입 연산자를 
                                                       // 안전하게 사용하기 위해 먼저 초기화 실시
    *this = std::move(other);
  }

  my_unique_ptr& operator=(const my_unique_ptr&) = delete;

  my_unique_ptr& operator=(my_unique_ptr&& other) noexcept {
    if (this != &other) {
      if (ptr_) {
        del_(ptr_);
      }
      ptr_ = other.release();
      del_ = std::move(other.del_);
    }
    return *this;
  }

  ~my_unique_ptr() {
    if (ptr_) {
      del_(ptr_);
    }
    ptr_ = nullptr;
  }

  void swap(my_unique_ptr& rhs) noexcept {
    using std::swap;
    swap(ptr_, rhs.ptr_);
    swap(del_, rhs.del_);
  }

  [[nodiscard]] T* get() {
    return ptr_;
  }
  
  [[nodiscard]] T* release() {
    T* temp = ptr_;
    ptr_ = nullptr;
    return temp;
  }

private:
  T* ptr_;
  D del_;
};







