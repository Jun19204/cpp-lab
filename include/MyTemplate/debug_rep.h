#pragma once

#include <sstream>
#include <string>

/* 특수화되지 않은 모든 타입을 출력 */
template <typename T>
std::string debug_rep(const T& t) {
  std::ostringstream ret;
  ret << t;
  return ret.str();
}

/*
 * 포인터 타입에 대해 특수화
 * const void*로 캐스트한 이유는 char*, const char* 인 경우에도 메모리 주소를 출력하기 위함
 */
template <typename T>
std::string debug_rep(T* ptr) {
  std::ostringstream ret;
  ret << "pointer: " << static_cast<const void*>(ptr); // 포인터에 저장된 메모리 주소를 출력
  if (ptr) {
    ret << " " << debug_rep(*ptr);
  } else {
    ret << " null pointer";
  }
  return ret.str();
}

/* string 전용 비템플릿 함수 */
std::string debug_rep(const std::string& s) {
  return '"' + s + '"';
}

/* char* 전용 비템플릿 함수 */
std::string debug_rep(char* ptr) {
  return debug_rep(ptr) + " " + debug_rep(std::string(ptr));
}
/* const char* 전용 비템플릿 함수 */
std::string debug_rep(const char* ptr) {
  return debug_rep(ptr) + " " + debug_rep(std::string(ptr));
}


