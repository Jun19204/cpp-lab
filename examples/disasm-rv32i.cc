#include "decoder/rv32i.h"
#include "parser/elf32_parser.h"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <span>
#include <string>
#include <vector>
#include <format>
#include <print>


namespace {
struct ToDecodeInstrInfo {
  uint32_t pc_;
  uint32_t bits_;
  size_t length_; // 2 or 4 bytes
};

template <typename T>
T readLe(std::span<const std::byte> data, size_t offset) {
  std::array<std::byte, sizeof(T)> raw{};
  std::copy_n(
      data.begin() + static_cast<ptrdiff_t>(offset), 
      sizeof(T), 
      raw.begin()
  );
  return std::bit_cast<T>(raw);
}
} // namespace


int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "사용법: disasm-rv32i <file_path>\n";
    return EXIT_FAILURE;
  }

  try {
    Elf32Parser parser{argv[1]};
    auto text_opt = parser.getText();
    if (!text_opt) {
      std::cerr << ".text/PT_LOAD 추출 실패\n";
      return EXIT_FAILURE;
    }
    
    const auto text_region = *text_opt;
    std::vector<ToDecodeInstrInfo> to_decode_list;
    size_t pos = 0;
    uint32_t pc = 0x00000000;
    
    while (pos < text_region.size()) {
      if (pos + sizeof(uint16_t) > text_region.size()) {
        std::cerr
            << "오류!: 잘못된 파일 크기입니다. 올바른 ISA인지 확인하세요.\n";
        return EXIT_FAILURE;
      }
      
      const auto half = readLe<uint16_t>(text_region, pos);
      uint32_t raw_instr = 0;
      size_t instr_len = 0;
      
      if ((half & 0x3) != 0x3) {
        // 16비트 압축 명령어 (RVC)
        raw_instr = static_cast<uint32_t>(half);
        instr_len = 2;
      } else {
        if (pos + sizeof(uint32_t) > text_region.size()) {
          std::cerr << "오류!: 32비트 명령어 바이트가 부족합니다.\n";
          return EXIT_FAILURE;
        }
        // 32비트 표준 명령어
        raw_instr = readLe<uint32_t>(text_region, pos);
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
    
    // 디스어셈블 결과 출력 (열 정렬 포함)
    std::println("-----------------------------------------------------");
    std::println(" PC 주소      바이너리 코드   디스어셈블 결과");
    std::println("-----------------------------------------------------");
    for (const auto& to_decode : to_decode_list) {
      auto decoded_instr = InstructionDecoder::decode(to_decode.bits_);
      
      const std::string hex_bits = (to_decode.length_ == 2)
                                 ? std::format("0x{:04x}", to_decode.bits_)
                                 : std::format("0x{:08x}", to_decode.bits_);
      std::println(
          "0x{:08x}   {:<13} {}",
          to_decode.pc_,
          hex_bits,
          decoded_instr->disassemble(to_decode.pc_)
      );
    }
  } catch (const std::exception& e) {
    std::cerr << "오류 발생: " << e.what() << "\n";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
