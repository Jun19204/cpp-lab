#pragma once

#include <stdexcept>
#include <string>
#include <vector>

// 에러 코드 상수 집합
enum class error_code {
  file_open_failed,
  file_read_failed,
  parse_failed,
  invalid_argument,
};

// 상황(context)
struct error_context {
  std::string key;
  std::string value;
};

// 실제 에러(error)
class error : public std::runtime_error {
public:
  error(error_code code, std::vector<error_context> contexts = {})
      : std::runtime_error(build_message(code, contexts)), code_{code},
        contexts_{std::move(contexts)} {}

  [[nodiscard]]
  constexpr error_code code() const noexcept {
    return code_;
  }

  [[nodiscard]]
  const std::vector<error_context> &contexts() const noexcept {
    return contexts_;
  }

private:
  static std::string build_message(error_code,
                                   const std::vector<error_context> &);

  error_code code_;
  std::vector<error_context> contexts_;
};
