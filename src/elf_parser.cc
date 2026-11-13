#include "parser/elf_parser.h"

#include <elf.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

/** 런타임 다형성을 위한 추상 기반 클래스.
 *  공개 객체는 이 타입을 통해 실제 ELF32/ELF64 구현에 호출을 위임한다.
 */
class ElfParser::ImplBase {
public:
  virtual ~ImplBase() = default;
  virtual auto extractTextSection() -> std::vector<std::uint8_t> = 0;
};

namespace {

// 파일 내 오프셋 및 길이 계산을 위한 공통 정수형.
using FileOffset = std::uint64_t;

// ELF 클래스별 구조체 차이는 Traits로 격리한다.
// 템플릿 알고리즘은 Ehdr/Shdr/Phdr 이름만 사용한다.
struct ELF32Traits {
  using Ehdr = Elf32_Ehdr;
  using Shdr = Elf32_Shdr;
  using Phdr = Elf32_Phdr;
  static constexpr unsigned char kExpectedClass = ELFCLASS32;
};

struct ELF64Traits {
  using Ehdr = Elf64_Ehdr;
  using Shdr = Elf64_Shdr;
  using Phdr = Elf64_Phdr;
  static constexpr unsigned char kExpectedClass = ELFCLASS64;
};

// 파일을 한 번만 열고, 선택된 구현 객체로 스트림 소유권을 이전한다.
auto openElf(const std::filesystem::path& filepath) -> std::ifstream {
  std::ifstream file(filepath, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("파일 열기 실패: " + filepath.string());
  }
  return file;
}

// 현재 열려 있는 스트림에서 파일 길이를 구하고 읽기 위치를 되돌린다.
auto getFileSize(std::ifstream& file) -> FileOffset {
  file.seekg(0, std::ios::end);
  if (!file) {
    throw std::runtime_error("파일 끝 위치 탐색 실패");
  }
  const auto end = file.tellg();
  if (end == std::ifstream::pos_type(-1)) {
    throw std::runtime_error("파일 크기 조회 실패");
  }
  const auto size = static_cast<std::streamoff>(end);
  if (size < 0) {
    throw std::runtime_error("유효하지 않은 파일 크기");
  }
  file.seekg(0, std::ios::beg);
  if (!file) {
    throw std::runtime_error("파일 시작 위치 탐색 실패");
  }
  return static_cast<FileOffset>(size);
}

// 파일의 길이가 호스트 벡터 크기 타입으로 표현되는지 검사한다.
auto checkedSize(FileOffset n) -> std::size_t {
  if (n > std::numeric_limits<std::size_t>::max()) {
    throw std::runtime_error("데이터 크기가 주소 공간을 초과합니다.");
  }
  return static_cast<std::size_t>(n);
}

// off + length 대신 뺄셈으로 계산하여 정수 오버플로를 피한다.
auto fitsInFile(FileOffset off, FileOffset length, FileOffset size) -> bool {
  return off <= size && length <= size - off;
}

template <typename Traits>
/** 실제 파싱 알고리즘을 공유하는 클래스 템플릿.
 *  Traits는 컴파일 타임 자료형 선택을, ImplBase는 런타임 위임을 담당한다.
 */
class ElfParserImpl final : public ElfParser::ImplBase {
public:
  explicit ElfParserImpl(std::ifstream file, FileOffset file_size)
      : file_(std::move(file)), file_size_(file_size) {
    validateAndReadHeader();
  }

  // ET_REL은 섹션 기반 추출만, ET_EXEC/ET_DYN은 섹션 우선 후 폴백.
  auto extractTextSection() -> std::vector<std::uint8_t> override {
    switch (ehdr_.e_type) {
      case ET_REL: {
        if (auto text = tryReadTextSection()) {
          return std::move(*text);
        }
        throw std::runtime_error(".text 섹션을 찾을 수 없습니다.");
      }
      case ET_EXEC:
      case ET_DYN:
        if (auto text = tryReadTextSection()) {
          return std::move(*text);
        }
        return readEntryPointSegment();
      default:
        throw std::runtime_error("지원하지 않는 ELF 파일 타입입니다.");
    }
  }

private:
  std::ifstream file_;
  FileOffset file_size_;
  typename Traits::Ehdr ehdr_{};

  // 모든 seek/read의 공통 관문: 경계와 스트림 표현 가능 범위를 검사한다.
  auto readAt(FileOffset offset, char* destination, FileOffset length) -> void {
    if (!fitsInFile(offset, length, file_size_)) {
      throw std::runtime_error("파일 읽기 범위가 유효하지 않습니다.");
    }
    if (length == 0) {
      return;
    }
    if (length > static_cast<FileOffset>(
                     std::numeric_limits<std::streamsize>::max()) ||
        offset > static_cast<FileOffset>(
                     std::numeric_limits<std::streamoff>::max())) {
      throw std::runtime_error("스트림에서 표현할 수 없는 파일 범위입니다.");
    }

    file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!file_) {
      throw std::runtime_error("파일 위치 이동 실패");
    }
    if (!file_.read(destination, static_cast<std::streamsize>(length))) {
      throw std::runtime_error("파일 데이터 읽기 실패");
    }
  }

  // ELF 구조체의 객체 표현에 파일 바이트를 복사한다.
  // 호스트 엔디언과 ELF 데이터 인코딩의 일치 여부는 앞서 검증한다.
  template <typename Header>
  auto readHeader(FileOffset offset) -> Header {
    static_assert(std::is_trivially_copyable_v<Header>);
    Header result{};
    readAt(offset, reinterpret_cast<char*>(&result), sizeof(result));
    return result;
  }

  // 유효한 파일 범위만 동적 바이트 버퍼로 읽는다.
  auto readBytes(FileOffset offset, FileOffset length)
      -> std::vector<std::uint8_t> {
    if (!fitsInFile(offset, length, file_size_)) {
      throw std::runtime_error("데이터 영역이 파일 범위를 벗어납니다.");
    }
    std::vector<std::uint8_t> bytes(checkedSize(length));
    if (!bytes.empty()) {
      readAt(offset, reinterpret_cast<char*>(bytes.data()), length);
    }
    return bytes;
  }

  /** 파일 헤더의 기본 불변 조건을 검증한다.
   *  매직·클래스·엔디언·버전·아키텍처·헤더 크기 및 테이블 범위를 확인한다.
   *  확장 번호 체계는 지원 범위 밖이므로 명시적으로 거부한다.
   */
  auto validateAndReadHeader() -> void {
    using Ehdr = typename Traits::Ehdr;
    if (file_size_ < sizeof(Ehdr)) {
      throw std::runtime_error("ELF 파일 헤더가 잘렸습니다.");
    }
    ehdr_ = readHeader<Ehdr>(0);
    const auto& id = ehdr_.e_ident;
    if (id[EI_MAG0] != ELFMAG0 || id[EI_MAG1] != ELFMAG1 ||
        id[EI_MAG2] != ELFMAG2 || id[EI_MAG3] != ELFMAG3) {
      throw std::runtime_error("ELF 매직 넘버가 일치하지 않습니다.");
    }
    if (id[EI_CLASS] != Traits::kExpectedClass) {
      throw std::runtime_error("ELF 클래스가 구현 타입과 다릅니다.");
    }
    const bool native_endian =
        (id[EI_DATA] == ELFDATA2LSB && std::endian::native == std::endian::little) ||
        (id[EI_DATA] == ELFDATA2MSB && std::endian::native == std::endian::big);
    if (!native_endian) {
      throw std::runtime_error("호스트 엔디언과 다른 ELF는 지원하지 않습니다.");
    }
    if (id[EI_VERSION] != EV_CURRENT || ehdr_.e_version != EV_CURRENT ||
        ehdr_.e_ehsize != sizeof(Ehdr)) {
      throw std::runtime_error("ELF 버전 또는 파일 헤더 크기가 유효하지 않습니다.");
    }
    if (ehdr_.e_machine != EM_RISCV) {
      throw std::runtime_error("RISC-V ELF 파일이 아닙니다.");
    }

    // 확장 헤더 번호 형식은 별도 해석이 필요하므로 명시적으로 거부한다.
    if (ehdr_.e_shstrndx == SHN_XINDEX || ehdr_.e_phnum == PN_XNUM ||
        (ehdr_.e_shnum == 0 && ehdr_.e_shoff != 0)) {
      throw std::runtime_error("확장 ELF 헤더 번호 형식은 지원하지 않습니다.");
    }
    if (ehdr_.e_shnum != 0) {
      if (ehdr_.e_shoff == 0 ||
          ehdr_.e_shentsize != sizeof(typename Traits::Shdr) ||
          !fitsInFile(ehdr_.e_shoff,
                      static_cast<FileOffset>(ehdr_.e_shnum) * ehdr_.e_shentsize,
                      file_size_)) {
        throw std::runtime_error("섹션 헤더 테이블이 유효하지 않습니다.");
      }
    }
    if (ehdr_.e_phnum != 0) {
      if (ehdr_.e_phoff == 0 ||
          ehdr_.e_phentsize != sizeof(typename Traits::Phdr) ||
          !fitsInFile(ehdr_.e_phoff,
                      static_cast<FileOffset>(ehdr_.e_phnum) * ehdr_.e_phentsize,
                      file_size_)) {
        throw std::runtime_error("프로그램 헤더 테이블이 유효하지 않습니다.");
      }
    }
  }

  // nullopt는 '섹션 테이블이 없거나 .text가 없음'을 의미한다.
  // 형식이 잘못된 경우에는 예외를 던져 폴백으로 숨기지 않는다.
  /** .shstrtab의 문자열을 통해 이름이 .text인 섹션을 탐색한다.
   *  nullopt: 테이블/해당 섹션이 없음; 예외: 손상된 구조.
   *  이 구분 덕분에 실제 파싱 오류가 폴백에 묻히지 않는다.
   */
  auto tryReadTextSection()
      -> std::optional<std::vector<std::uint8_t>> {
    using Shdr = typename Traits::Shdr;
    if (ehdr_.e_shnum == 0) {
      return std::nullopt;
    }
    if (ehdr_.e_shstrndx == SHN_UNDEF) {
      return std::nullopt;
    }
    if (ehdr_.e_shstrndx >= ehdr_.e_shnum) {
      throw std::runtime_error("섹션 이름 테이블 인덱스가 유효하지 않습니다.");
    }

    // e_shoff + index * e_shentsize: i번째 섹션 헤더의 파일 위치.
    const auto sectionAt = [this](FileOffset index) -> Shdr {
      return readHeader<Shdr>(ehdr_.e_shoff + index * ehdr_.e_shentsize);
    };
    const Shdr names_header = sectionAt(ehdr_.e_shstrndx);
    if (names_header.sh_type != SHT_STRTAB) {
      throw std::runtime_error("섹션 이름 테이블의 타입이 올바르지 않습니다.");
    }
    const auto names = readBytes(names_header.sh_offset, names_header.sh_size);

    for (FileOffset i = 0; i < ehdr_.e_shnum; ++i) {
      const Shdr section = sectionAt(i);
      if (section.sh_name >= names.size()) {
        throw std::runtime_error("섹션 이름 오프셋이 테이블 범위를 벗어납니다.");
      }
      const char* begin = reinterpret_cast<const char*>(names.data()) +
                          section.sh_name;
      const char* end = reinterpret_cast<const char*>(names.data()) + names.size();
      // 종료 문자를 테이블 안에서만 탐색하여 범위 밖 읽기를 방지한다.
      const char* terminator = std::find(begin, end, '\0');
      if (terminator == end) {
        throw std::runtime_error("종료되지 않은 ELF 섹션 이름입니다.");
      }
      if (std::string_view(begin, static_cast<std::size_t>(terminator - begin))
          != ".text") {
        continue;
      }
      if (section.sh_type != SHT_PROGBITS) {
        throw std::runtime_error(".text 섹션의 타입이 SHT_PROGBITS가 아닙니다.");
      }
      return readBytes(section.sh_offset, section.sh_size);
    }
    return std::nullopt;
  }

  // .text를 얻지 못하는 실행 파일에서는 진입점부터 파일에 적재된
  // 실행 가능 PT_LOAD의 끝까지 읽는다. 이는 .text 전체와 다르다.
  /** 진입점(e_entry)이 포함된 실행 가능 PT_LOAD를 탐색한다.
   *  가상 주소의 상대 위치(delta)를 파일 오프셋에 대응시켜
   *  진입점부터 p_filesz의 끝까지 읽는다(전체 .text가 아님).
   */
  auto readEntryPointSegment() -> std::vector<std::uint8_t> {
    using Phdr = typename Traits::Phdr;
    if (ehdr_.e_phnum == 0) {
      throw std::runtime_error("프로그램 헤더 테이블이 존재하지 않습니다.");
    }
    for (FileOffset i = 0; i < ehdr_.e_phnum; ++i) {
      const Phdr phdr = readHeader<Phdr>(
          ehdr_.e_phoff + i * ehdr_.e_phentsize);
      if (phdr.p_type != PT_LOAD || (phdr.p_flags & PF_X) == 0) {
        continue;
      }
      if (phdr.p_filesz > phdr.p_memsz ||
          !fitsInFile(phdr.p_offset, phdr.p_filesz, file_size_)) {
        throw std::runtime_error("실행 세그먼트의 파일 범위가 유효하지 않습니다.");
      }
      // e_entry - p_vaddr를 사용하여 주소 덧셈 오버플로를 피한다.
      if (ehdr_.e_entry < phdr.p_vaddr) {
        continue;
      }
      const FileOffset delta =
          static_cast<FileOffset>(ehdr_.e_entry) - phdr.p_vaddr;
      if (delta >= phdr.p_filesz) {
        continue;
      }
      // 앞선 PT_LOAD 파일 범위 및 delta < p_filesz 검사로 안전한 합.
      const FileOffset file_offset = phdr.p_offset + delta;
      return readBytes(file_offset, phdr.p_filesz - delta);
    }
    throw std::runtime_error(
        "진입점이 포함된 파일 기반 실행 PT_LOAD 세그먼트를 찾지 못했습니다.");
  }
};

} // namespace

// 사용자 진입점: ELF 식별자의 EI_CLASS로 내부 파서를 선택한다.
// 파일을 두 번 열지 않고 스트림 소유권을 구현 객체로 넘긴다.
ElfParser::ElfParser(const std::filesystem::path& filepath) {
  auto file = openElf(filepath);
  const FileOffset file_size = getFileSize(file);
  if (file_size < EI_NIDENT) {
    throw std::runtime_error("ELF 식별 헤더가 잘렸습니다.");
  }
  std::array<unsigned char, EI_NIDENT> ident{};
  if (!file.read(reinterpret_cast<char*>(ident.data()),
                 static_cast<std::streamsize>(ident.size()))) {
    throw std::runtime_error("ELF 식별 헤더 읽기 실패");
  }
  if (ident[EI_MAG0] != ELFMAG0 || ident[EI_MAG1] != ELFMAG1 ||
      ident[EI_MAG2] != ELFMAG2 || ident[EI_MAG3] != ELFMAG3) {
    throw std::runtime_error("ELF 매직 넘버가 일치하지 않습니다.");
  }
  switch (ident[EI_CLASS]) {
    case ELFCLASS32:
      impl_ = std::make_unique<ElfParserImpl<ELF32Traits>>(
          std::move(file), file_size);
      break;
    case ELFCLASS64:
      impl_ = std::make_unique<ElfParserImpl<ELF64Traits>>(
          std::move(file), file_size);
      break;
    default:
      throw std::runtime_error("지원하지 않는 ELF 클래스입니다.");
  }
}

// ImplBase의 정의가 보이는 위치에서 특수 멤버 함수를 정의한다.
ElfParser::~ElfParser() = default;
ElfParser::ElfParser(ElfParser&&) noexcept = default;
ElfParser& ElfParser::operator=(ElfParser&&) noexcept = default;

// 이동 후 객체의 빈 상태를 명시적으로 검사한 다음 위임한다.
auto ElfParser::extractTextSection() -> std::vector<std::uint8_t> {
  if (!impl_) {
    throw std::logic_error("이동 후 비어 있는 ElfParser입니다.");
  }
  return impl_->extractTextSection();
}
