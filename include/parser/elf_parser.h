#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

/* ELF 파일의 .text 또는 진입점 이후 실행 세그먼트 바이트를 읽는다. */
class ElfParser {
public:
  explicit ElfParser(const std::filesystem::path& filepath);
  ~ElfParser();
  ElfParser(ElfParser&&) noexcept;
  ElfParser& operator=(ElfParser&&) noexcept;
  ElfParser(const ElfParser&) = delete;
  ElfParser& operator=(const ElfParser&) = delete;
  // 이동된 객체에서 호출하면 std::logic_error를 던진다.
  [[nodiscard]] auto extractTextSection() -> std::vector<std::uint8_t>;
  // 구체적인 구현체가 상속할 타입. 정의는 .cc 내부에 둔다.
  class ImplBase;

private:
  std::unique_ptr<ImplBase> impl_;
};

#endif // ELF_PARSER_H
