#include "StrVec/StrVec.h"
#include <algorithm>
#include <memory>
#include <utility>

using namespace std;

void StrVec::push_back(const string& s) {
  chk_n_alloc();
  allocator_traits<decltype(alloc)>
    ::construct(alloc, first_free++, s);
}

void StrVec::push_back(string&& s) { 
  chk_n_alloc();
  allocator_traits<decltype(alloc)>
    ::construct(alloc, first_free++, std::move(s));
}

template <typename... Args>
void StrVec::emplace_back(Args&&... args) {
  chk_n_alloc();
  allocator_traits<decltype(alloc)>
    ::construct(alloc, first_free++, std::forward<Args>(args)...);
}

auto StrVec::alloc_n_copy(const string* b, const string* e) 
-> pair<string*, string*> {
  auto* data = alloc.allocate(static_cast<size_t>(e - b));
  return make_pair(data, uninitialized_copy(b, e, data));
}

void StrVec::free() {
  if (elements != nullptr) {
    for_each(make_reverse_iterator(first_free), make_reverse_iterator(elements),
             [](const string& s) -> void {
               allocator_traits<decltype(alloc)>::destroy(alloc, addressof(s));
             });
    alloc.deallocate(elements, static_cast<size_t>(cap - elements));
  }
}

// 복사 제어 - 복사 생성자
StrVec::StrVec(const StrVec& s) {
  auto newdata = alloc_n_copy(s.begin(), s.end());
  elements = newdata.first;
  first_free = cap = newdata.second;
}

// 복사 제어 - 이동 생성자
StrVec::StrVec(StrVec&& s) noexcept
  : elements(s.elements)
  , first_free(s.first_free)
  , cap(s.cap) {
  s.elements = s.first_free = s.cap = nullptr;
}

// 복사 제어 - 복사 대입 연산자
auto StrVec::operator=(const StrVec& rhs) & -> StrVec& {
  if (this == &rhs) {
    return *this;
  }
  auto data = alloc_n_copy(rhs.begin(), rhs.end());
  free();
  elements = data.first;
  first_free = cap = data.second;
  return *this;
}

// 복사 제어 - 이동 대입 연산자
auto StrVec::operator=(StrVec&& rhs) & noexcept -> StrVec& {
  if (this != &rhs) {
    free();
    elements = rhs.elements;
    first_free = rhs.first_free;
    cap = rhs.cap;
    rhs.elements = rhs.first_free = rhs.cap = nullptr;
  }
  return *this;
}

// 복사 제어 - 소멸자
StrVec::~StrVec() { free(); }

void StrVec::reallocate() {
  auto newcapacity = (size() != 0) ? 2 * size() : 1;
  auto* first = alloc.allocate(newcapacity);
  auto* last = uninitialized_copy(make_move_iterator(begin()),
                                 make_move_iterator(end()), first);

  free();
  elements = first;
  first_free = last;
  cap = elements + newcapacity;
}


// 연산자 오버로딩 - 대입
auto StrVec::operator=(initializer_list<string> il) -> StrVec& {
  auto data = alloc_n_copy(il.begin(), il.end());
  free();
  elements = data.first;
  first_free = cap = data.second;
  return *this;
}

// 연산자 오버로딩 - 첨자










