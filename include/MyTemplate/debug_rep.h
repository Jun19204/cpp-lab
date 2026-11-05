#pragma once

#include <ostream>
#include <sstream>
#include <string>

/* 특수화되지 않은 모든 타입을 출력 */
template <typename T> auto debug_rep(const T &t) -> std::string {
  std::ostringstream ret;
  ret << t;
  return ret.str();
}

/*
 * 포인터 타입에 대해 오버로드
 * const void*로 캐스트한 이유는 char*, const char* 인 경우에도 메모리 주소를
 * 출력하기 위함
 */
template <typename T> auto debug_rep(T *ptr) -> std::string {
  std::ostringstream ret;
  ret << "pointer: "
      << static_cast<const void *>(ptr); // 포인터에 저장된 메모리 주소를 출력
  if (ptr) {
    ret << " " << debug_rep(*ptr);
  } else {
    ret << " null pointer";
  }
  return ret.str();
}

/* string 전용 비템플릿 함수 */
auto debug_rep(const std::string& s) -> std::string { return '"' + s + '"'; }

/* const char* 전용 비템플릿 함수 */
auto debug_rep(const char *ptr) -> std::string {
  if (ptr == nullptr) {
    return "pointer: null pointer";
  }
  return debug_rep<const char>(ptr) + " " + debug_rep(std::string(ptr));
}

/* char* 전용 비템플릿 함수 */
auto debug_rep(char *ptr) -> std::string {
  if (ptr == nullptr) {
    return "pointer: null pointer";
  }
  return debug_rep<char>(ptr) + " " + debug_rep(std::string(ptr));
}

/*
 * 매개변수 묶음을 확장하여 재귀적으로 호출
 */
template <typename T, typename... Args>
std::ostream& print(std::ostream& os, const T& t, const Args&... pack) {
  return print(os, pack...);
}

/*
 * 매개변수 묶음 확장을 통한 디버깅용 함수
 */
template <typename... Args>
std::ostream& errorMsg(std::ostream& os, const Args&... pack) {
  // print(os, debug_rep(a1), debug_rep(a2), ..., debug_rep(n))
  return print(os, debug_rep(pack)...);
}
