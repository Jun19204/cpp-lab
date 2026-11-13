#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <cstdint>
#include <filesystem>
#include <memory>
#include <vector>

/** ELF 파일의 .text 또는 진입점 이후 실행 세그먼트 바이트를 읽는다. */
/**
 * @brief ELF 파일의 명령어 바이트를 읽는 공개 진입점.
 *
 * 구현 클래스의 타입과 파일 처리 로직은 .cpp 안에 두고,
 * 사용자는 ELF32/ELF64의 차이 없이 동일한 API를 호출한다.
 *
 * @note .text가 없는 실행 파일에서 반환하는 바이트는
 *       진입점 이후 실행 가능 PT_LOAD의 파일 데이터이며,
 *       항상 정확한 .text 섹션을 의미하지는 않는다.
 */
class ElfParser {
public:
  // 파일 식별 헤더를 읽어 ELF32/ELF64 구현 중 하나를 선택한다.
  explicit ElfParser(const std::filesystem::path& filepath);
  // ImplBase가 완전한 타입인 .cpp에서 소멸자를 정의한다.
  ~ElfParser();

  // 내부 unique_ptr의 소유권은 이전 가능하지만 복사는 허용하지 않는다.
  ElfParser(ElfParser&&) noexcept;
  ElfParser& operator=(ElfParser&&) noexcept;
  ElfParser(const ElfParser&) = delete;
  ElfParser& operator=(const ElfParser&) = delete;

  // 이동된 객체에서 호출하면 std::logic_error를 던진다.
  /** @return 추출된 바이트의 독립적인 복사본.
   *  @throws std::runtime_error 입력 파일이 유효하지 않거나 추출이 실패한 경우.
   *  @throws std::logic_error 이동 후 비어 있는 파서에서 호출한 경우.
   */
  [[nodiscard]] auto extractTextSection() -> std::vector<std::uint8_t>;

  // 구체적인 구현체가 상속할 타입. 정의는 .cpp 내부에 둔다.
  // 구현의 구체적인 정의는 숨기지만 타입 이름은 공개 영역에 노출된다.
  class ImplBase;

private:
  std::unique_ptr<ImplBase> impl_;
};

#endif // ELF_PARSER_H
