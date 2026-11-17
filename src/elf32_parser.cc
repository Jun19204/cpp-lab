#include "parser/elf32_parser.h"

#include <algorithm>
#include <array>
#include <bit>
#include <filesystem>
#include <fstream>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>

Elf32Parser::Elf32Parser(const std::filesystem::path& filepath) {
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error(
        "파일(" + filepath.filename().string() + ") 열기 실패");
  }

  const auto size = file.tellg();
  file.seekg(0, std::ios::beg);

  buffer_.resize(static_cast<size_t>(size));
  if (!file.read(reinterpret_cast<char*>(buffer_.data()), size)) {
    throw std::runtime_error(
        "파일(" + filepath.filename().string() + ") 읽기 실패");
  }

  validateHeader();
}


auto Elf32Parser::getSection(std::string_view name) const 
  -> std::optional<std::span<const std::byte>> {
  const auto ehdr = readAt<Elf32_Ehdr>(0);

  if (ehdr.e_shoff == 0 || ehdr.e_shnum == 0 ||
      ehdr.e_shstrndx >= ehdr.e_shnum) {
    return std::nullopt;
  }

  const size_t strtab_hdr_offset =
      static_cast<size_t>(ehdr.e_shoff) +
      (static_cast<size_t>(ehdr.e_shstrndx) * ehdr.e_shentsize);
  const auto strtab_hdr = readAt<Elf32_Shdr>(strtab_hdr_offset);
  const auto strtab_span = slice(strtab_hdr.sh_offset, strtab_hdr.sh_size);

  for (size_t i = 0; i < ehdr.e_shnum; ++i) {
    const size_t shdr_offset = static_cast<size_t>(ehdr.e_shoff) 
                             + (i * ehdr.e_shentsize);
    const auto shdr = readAt<Elf32_Shdr>(shdr_offset);
    
    if (getStringAt(strtab_span, shdr.sh_name) == name) {
      return slice(shdr.sh_offset, shdr.sh_size);
    }
  }

  return std::nullopt;
}


auto Elf32Parser::getSegment(std::string_view name) const 
  -> std::optional<std::span<const std::byte>> {
  const auto ehdr = readAt<Elf32_Ehdr>(0);

  if (ehdr.e_phoff == 0 || ehdr.e_phnum == 0) {
    return std::nullopt;
  }

  for (size_t i = 0; i < ehdr.e_phnum; ++i) {
    const size_t phdr_offset = static_cast<size_t>(ehdr.e_phoff) 
                             + (i * ehdr.e_phentsize);
    const auto phdr = readAt<Elf32_Phdr>(phdr_offset);
      
    if (matchSegment(phdr, name)) {
      return slice(phdr.p_offset, phdr.p_filesz);
    }
  }

  return std::nullopt;
}


auto Elf32Parser::getText() const
  -> std::optional<std::span<const std::byte>> {
  if (auto sec = getSection(".text")) {
    return sec;
  }
  return getSegment("LOAD_RX");
}


bool Elf32Parser::matchSegment(const Elf32_Phdr& phdr,std::string_view name) {
  if (name == "LOAD_RX" || name == ".text") {
    return phdr.p_type == PT_LOAD && (phdr.p_flags & PF_X) != 0;
  }
  if (name == "LOAD_RW" || name == ".data") {
    return (phdr.p_type == PT_LOAD) && 
           (phdr.p_flags & PF_W) != 0 &&
           (phdr.p_flags & PF_X) == 0;
  }
  if (name == "LOAD_R" || name == ".rodata") {
    return phdr.p_type == PT_LOAD && phdr.p_flags == PF_R;
  }
  if (name == "INTERP" || name == ".interp") {
    return phdr.p_type == PT_INTERP;
  }
  if (name == "DYNAMIC" || name == ".dynamic") {
    return phdr.p_type == PT_DYNAMIC;
  }
  if (name == "NOTE") {
    return phdr.p_type == PT_NOTE;
  }
  return false;
}


template <typename T> 
requires std::is_trivially_copyable_v<T>
T Elf32Parser::readAt(size_t offset) const {
  const auto bytes = slice(offset, sizeof(T));
  std::array<std::byte, sizeof(T)> raw{};
  std::copy(bytes.begin(), bytes.end(), raw.begin());
  return std::bit_cast<T>(raw);
}


auto Elf32Parser::slice(size_t offset, size_t count) const 
  -> std::span<const std::byte> {
  if (offset > buffer_.size() || count > buffer_.size() - offset) {
    throw std::out_of_range("ELF 파일의 유효 범위를 벗어난 접근입니다.");
  }
  return std::span{buffer_}.subspan(offset, count);
}


auto Elf32Parser::getStringAt(std::span<const std::byte> strtab, size_t offset) 
  -> std::string_view {
  if (offset >= strtab.size()) {
    return {};
  }
  const auto sub = strtab.subspan(offset);
  const auto iter = std::find(sub.begin(), sub.end(), std::byte{0});
  const auto len = static_cast<size_t>(std::distance(sub.begin(), iter));
  return {reinterpret_cast<const char*>(sub.data()), len};
}


void Elf32Parser::validateHeader() const {
  const auto ehdr = readAt<Elf32_Ehdr>(0);
  
  constexpr std::array<unsigned char, 4> magic{0x7F, 'E', 'L', 'F'};
  if (!std::equal(ehdr.e_ident, ehdr.e_ident + 4, magic.begin())) {
    throw std::runtime_error("유효한 ELF 파일이 아닙니다.");
  }
  
  if (ehdr.e_ident[4] != 1) { // ELFCLASS32
    throw std::runtime_error("32비트 ELF 파일만 지원합니다.");
  }
  
  const bool is_little_endian = (ehdr.e_ident[5] == 1);
  if ((std::endian::native == std::endian::little) != is_little_endian) {
    throw std::runtime_error("호스트와 ELF 파일의 엔디안이 일치하지 않습니다.");
  }
}

