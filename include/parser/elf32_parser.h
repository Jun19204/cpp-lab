#ifndef ELF32_PARSER_H
#define ELF32_PARSER_H

#include <cstring>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

#if __has_include(<elf.h>)
#include <elf.h>
#else
#define ELF_H
struct Elf32_Ehdr;
struct Elf32_Shdr;
struct Elf32_Phdr;
#endif // __has_include(<elf.h>)


template <typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;


class Elf32Parser {
public:
  explicit Elf32Parser(const std::filesystem::path& filepath);

  // 1. 섹션 이름으로 추출 (예: ".text", ".rodata", ".data")
  [[nodiscard]] 
  auto getSection(std::string_view name) const
    -> std::optional<std::span<const std::byte>>;
 
  // 2. 세그먼트 식별 이름으로 추출 (예: "LOAD_RX", "LOAD_RW", "INTERP", "DYNAMIC")
  [[nodiscard]] 
  auto getSegment(std::string_view name) const
    -> std::optional<std::span<const std::byte>>;
  
  // 3. 사용자 편의용 통합 함수 (.text 섹션 우선 -> 없으면 실행 세그먼트 폴백)
  [[nodiscard]] 
  auto getText() const
    -> std::optional<std::span<const std::byte>>;

private:
  std::vector<std::byte> buffer_;

  // 문자열 이름과 Program Header의 타입/플래그 매칭
  [[nodiscard]] 
  static bool match_segment(const Elf32_Phdr& phdr, std::string_view name);
  
  template <TriviallyCopyable T> 
  [[nodiscard]] 
  T readAt(size_t offset) const;
  
  [[nodiscard]] 
  auto slice(size_t offset, size_t count) const
    -> std::span<const std::byte>;
 
  [[nodiscard]] 
  static auto getStringAt (std::span<const std::byte> strtab, size_t offset) 
    -> std::string_view;
    
  void validateHeader() const;
 };


#ifdef ELF_H
struct Elf32_Ehdr {
  unsigned char e_ident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint32_t e_entry;
  uint32_t e_phoff;
  uint32_t e_shoff;
  uint32_t e_flags;
  uint16_t ehsize;
  uint16_t e_phentsize;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
};

struct Elf32_Shdr {
  uint32_t sh_name;
  uint32_t sh_type;
  uint32_t sh_flags;
  uint32_t sh_addr;
  uint32_t sh_offset;
  uint32_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
  uint32_t sh_addralign;
  uint32_t sh_entsize;
};

struct Elf32_Phdr {
  uint32_t p_type;
  uint32_t p_offset;
  uint32_t p_vaddr;
  uint32_t p_paddr;
  uint32_t p_filesz;
  uint32_t p_memsz;
  uint32_t p_flags;
  uint32_t p_align;
};
#endif // ELF_H

#endif // ELF32_PARSER_H
