#ifndef PARSER_H
#define PARSER_H

#include <array>
#include <cstdint>
#include <elf.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// 1. 공통 인터페이스 (팩토리가 다형적으로 반환하기 위함)
class IElfParser {
public:
  virtual ~IElfParser() = default;
  virtual auto extractTextSection() -> std::vector<uint8_t> = 0;
};



// 2. 32비트/64비트 트레이츠 (시스템 표준 구조체 매핑)
struct ELF32Traits {
  using Ehdr = Elf32_Ehdr;
  using Shdr = Elf32_Shdr;
  using Phdr = Elf32_Phdr;
  static constexpr uint8_t kExpectedClass = ELFCLASS32;
};
struct ELF64Traits {
  using Ehdr = Elf64_Ehdr;
  using Shdr = Elf64_Shdr;
  using Phdr = Elf64_Phdr;
  static constexpr uint8_t kExpectedClass = ELFCLASS64;
};



// 3. 구체적인 템플릿 파서 클래스
template <typename Traits> class ElfParser : public IElfParser {
public:
  explicit ElfParser(const std::filesystem::path &filepath)
      : file_(filepath, std::ios::binary) {
    if (!file_.is_open()) {
      throw std::runtime_error("파일 열기 실패: " + filepath.string());
    }
    validateAndReadHeader();
  }

  auto extractTextSection() -> std::vector<uint8_t> override {
    // 파일 타입(ET_REL vs ET_EXEC/ET_DYN)에 따른 분기 처리 구현 영역
    switch (ehdr_.e_type) {
      case ET_REL:
        return parseRelocatableText();
      case ET_EXEC:
      case ET_DYN:
        return parseExecutableText();
      default:
        throw std::runtime_error("지원하지 않는 ELF 파일 타입입니다. (e_type)");
    }
  }

private:
  std::ifstream file_;
  typename Traits::Ehdr ehdr_;

  void validateAndReadHeader() {
    // 매직 넘버 검증 및 32/64비트 클래스 확인 로직
    std::array<unsigned char, EI_NIDENT> ident;
    file_.read(reinterpret_cast<char *>(ident.data()), EI_NIDENT);
    
    if (ident[EI_MAG0] != ELFMAG0 || ident[EI_MAG1] != ELFMAG1 ||
        ident[EI_MAG2] != ELFMAG2 || ident[EI_MAG3] != ELFMAG3) {
      throw std::runtime_error("유효한 ELF 파일이 아닙니다 (매직 넘버 불일치)");
    }
    if (ident[EI_CLASS] != Traits::kExpectedClass) {
      throw std::runtime_error("ELF 클래스가 트레이츠와 일치하지 않습니다.");
    }
    
    // 파일 포인터를 다시 처음으로 돌려 전체 ELF 헤더 읽기
    file_.seekg(0, std::ios::beg);
    file_.read(reinterpret_cast<char *>(&ehdr_), sizeof(ehdr_));
  }

  auto parseRelocatableText() -> std::vector<uint8_t>;
  auto parseExecutableText() -> std::vector<uint8_t>;
};


template <typename Traits>
auto ElfParser<Traits>::parseRelocatableText()
  -> std::vector<uint8_t> 
{
  using Shdr = typename Traits::Shdr;

  if (ehdr_.e_shoff == 0 || ehdr_.e_shnum == 0) {
    throw std::runtime_error("목적 파일에 섹션 헤더 테이블이 존재하지 않습니다.");
  }
  if (ehdr_.e_shstrndx == SHN_UNDEF || ehdr_.e_shstrndx >= ehdr_.e_shnum) {
    throw std::runtime_error("유효하지 않은 섹션 이름 스트링 테이블 인덱스입니다.");
  }

  auto shstrtab_hdr_offset = ehdr_.e_shoff
                           + (static_cast<size_t>(ehdr_.e_shstrndx)
                           * ehdr_.e_shentsize);
  file_.seekg(shstrtab_hdr_offset, std::ios::beg);

  Shdr shstrtab_shdr;
  if (!file_.read(reinterpret_cast<char*>(&shstrtab_shdr), sizeof(Shdr))) {
    throw std::runtime_error("섹션 이름 스트링 테이블 헤더 읽기 실패");
  }

  std::vector<char> shstrtab(shstrtab_shdr.sh_size);
  file_.seekg(shstrtab_shdr.sh_offset, std::ios::beg);
  if (!file_.read(shstrtab.data(), shstrtab_shdr.sh_size)) {
    throw std::runtime_error("식션 이름 스트링 테이블 데이터 읽기 실패");
  }

  for (size_t i = 0; i < ehdr_.e_shnum; ++i) {
    auto shdr_offset = ehdr_.e_shoff + (i * ehdr_.e_shentsize);
    file_.seekg(shdr_offset, std::ios::beg);
    
    Shdr shdr;
    if (!file_.read(reinterpret_cast<char*>(&shdr), sizeof(Shdr))) {
      throw std::runtime_error("섹션 헤더 일기 실패 (인덱스: " + std::to_string(i) + ")");
    }
    
    if (shdr.sh_name >= shstrtab.size()) {
      continue; // 잘못된 오프셋 방어
    }
    
    const char* sec_name = &shstrtab[shdr.sh_name];
    if (std::string_view(sec_name) == ".text") {
      if (shdr.sh_size == 0) {
        return {}; //코드가 없는 빈 .text 섹션
      }
      
      std::vector<uint8_t> text_bytes(shdr.sh_size);
      file_.seekg(shdr.sh_offset, std::ios::beg);
      if (!file_.read(reinterpret_cast<char*>(text_bytes.data()), shdr.sh_size)) {
        throw std::runtime_error(".text 섹션 바이너리 데이터 읽기 실패");
      }
      
      return text_bytes;
    }
  }
  throw std::runtime_error("목적 파일에서 .text 섹션을 찾을 수 없습니다.");
}

template <typename Traits>
auto ElfParser<Traits>::parseExecutableText()
  -> std::vector<uint8_t> 
{
  using Phdr = typename Traits::Phdr;

  if (ehdr_.e_phoff == 0 || ehdr_.e_phnum == 0) {
    throw std::runtime_error("실행 파일/공유 라이브러리에 프로그램 헤더 테이블이 존재하지 않습니다.");
  }

  for (size_t i = 0; i < ehdr_.e_phnum; ++i) {
    auto phdr_offset = ehdr_.e_phoff + (i * ehdr_.e_phentsize);
    file_.seekg(phdr_offset, std::ios::beg);
    
    Phdr phdr;
    if (!file_.read(reinterpret_cast<char*>(&phdr), sizeof(Phdr))) {
      throw std::runtime_error("프로그램 헤더 읽기 실패 (인덱스: " + std::to_string(i) + ")");
    }
    
    if (phdr.p_type == PT_LOAD && (phdr.p_flags & PF_X)) {
      if (phdr.p_filesz == 0) {
        continue;
      }
      
      std::vector<uint8_t> text_bytes(phdr.p_filesz);
      file_.seekg(phdr.p_offset, std::ios::beg);
      if (!file_.read(reinterpret_cast<char*>(text_bytes.data()), phdr.p_filesz)) {
        throw std::runtime_error("실행 가능한 세그먼트 바이너리 데이터 읽기 실패");
      }
      return text_bytes;
    }
  }

  throw std::runtime_error("실행 가능한 세그먼트(PT_LOAD + PF_X)를 찾을 수 없습니다.");
}




// 4. 팩토리 함수: 런타임에 파일 헤더를 살짝 읽어 적절한 파서 객체를 생성
inline std::unique_ptr<IElfParser>
createElfParser(const std::filesystem::path &filepath) {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("파일 열기 실패: " + filepath.string());
  }

  std::array<unsigned char, EI_NIDENT> ident;
  if (!file.read(reinterpret_cast<char *>(ident.data()), EI_NIDENT)) {
    throw std::runtime_error("ELF 헤더 식별자 읽기 실패");
  }

  switch (ident[EI_CLASS]) {
    case ELFCLASS32:
      return std::make_unique<ElfParser<ELF32Traits>>(filepath);
    case ELFCLASS64:
      return std::make_unique<ElfParser<ELF64Traits>>(filepath);
    default:
      throw std::runtime_error("알 수 없는 ELF 클래스 타입입니다.");
  }
}

#endif
