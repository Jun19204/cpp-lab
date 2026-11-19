#include "decoder/rv32i.h"
#include "parser/elf32_parser.h"

#include <array>
#include <string>
#include <vector>
#include <span>
#include <bit>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <format>
#include <print>
#include <exception>
#include <stdexcept>
#include <concepts>
#include <type_traits>


namespace {

struct ToDecodeInstrInfo {
  const uint32_t pc_;
  const uint32_t bits_;
  const size_t length_;
};

template <typename T>
concept UnsignedInteger 
    = std::unsigned_integral<T> &&
      (!std::same_as<std::remove_cv_t<T>, bool>);

template <UnsignedInteger T> 
[[nodiscard]]
T readLE(std::span<const std::byte> data, size_t offset) {
  if (offset > data.size() || sizeof(T) > data.size() - offset) {
    throw std::out_of_range(
        std::format("readLE 범위 초과: offset={}, size={}, data_size={}",
                                        offset, sizeof(T), data.size())
    );
  }
  std::array<std::byte, sizeof(T)> raw_buffer{};
  std::copy_n(
      data.begin() + static_cast<ptrdiff_t>(offset), 
      sizeof(T), 
      raw_buffer.begin()
  );
  return std::bit_cast<T>(raw_buffer);
}

} // namespace



int main(int argc, char** argv) {
  if (argc != 2) {
    std::println(stderr, "<사용법>");
    std::println(stderr, "Linux/Unix: ./경로/disasm-rv32i <ELF 파일>");
    std::println(stderr, "Windows: 경로\\disasm-rv32i.exe <ELF 파일>");
    return EXIT_FAILURE;
  }

  try {
    Elf32Parser parser{argv[1]};
    auto text_opt = parser.getTextRegion();
    if (!text_opt) {
      std::println(stderr, "Text영역 추출 실패");
      return EXIT_FAILURE;
    }
    
    const auto text_region = *text_opt;
    std::vector<ToDecodeInstrInfo> to_decode_list;
    size_t pos = 0;
    uint32_t pc = text_region.begin_vma_;
    
    while (pos < text_region.data_.size()) {
      uint32_t raw_instr = 0;
      size_t instr_len = 0;
      const auto half = readLE<uint16_t>(text_region.data_, pos);
      
      if ((half & 0x3) != 0x3) { // 16비트 압축 명령어
        raw_instr = static_cast<uint32_t>(half);
        instr_len = 2;
      } else { // 32비트 표준 명령어
        raw_instr = readLE<uint32_t>(text_region.data_, pos);
        instr_len = 4;
      }
      
      to_decode_list.push_back(ToDecodeInstrInfo{
          .pc_ = pc,
          .bits_ = raw_instr,
          .length_ = instr_len,
      });
      
      pos += instr_len;
      pc += static_cast<uint32_t>(instr_len);
    }
    
    // 디스어셈블 결과 출력
    std::println("---------------------------------------------------------");
    std::println("   PC(VMA)      바이너리       디스어셈블 결과");
    std::println("---------------------------------------------------------");
    for (const auto& to_decode : to_decode_list) {
      auto decoded_instr = Rv32iDecoder::decode(to_decode.bits_);
      
      const std::string hex_bits = (to_decode.length_ == 2)
                                 ? std::format("0x{:04x}", to_decode.bits_)
                                 : std::format("0x{:08x}", to_decode.bits_);
      std::println(
          " 0x{:08x}    {:<13} {}",
          to_decode.pc_,
          hex_bits,
          decoded_instr->disassemble(to_decode.pc_)
      );
    }
  } catch (const std::exception& e) {
    std::println(stderr, "런타임 오류: {}", e.what());
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
