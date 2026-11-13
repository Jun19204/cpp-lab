#include "parser/elf_parser.h"
#include "decoder/rv32i.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>


namespace  {
// PC 주소와 바이너리 코드를 담은 구조체
struct ToDecodeInstruction {
  uint32_t pc_;
  uint32_t bits_;
  size_t length_; // 2 or 4 bytes
};
} // namespace


int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "사용법: disasm-rv32i <file_path>\n";
    return EXIT_FAILURE;
  }

  std::string file_path(argv[1]);
  ElfParser parser{file_path};
  auto text_section = parser.extractTextSection();

  std::vector<ToDecodeInstruction> to_decode_list;
  size_t pos = 0;
  uint32_t pc = 0x00000000; // 시작 주소 (필요시 e_entry 기반 주소로)

  // 가변 길이 바이트 patch 및 parsing
  while (pos < text_section.size()) {
    if (pos + sizeof(uint16_t) > text_section.size()) {
      std::cerr << "오류!: 잘못된 파일 크기 입니다. 올바른 ISA인지 확인하세요.\n";
      return EXIT_FAILURE;
    }
    
    uint16_t half = 0;
    std::memcpy(&half, &text_section[pos], sizeof(uint16_t));
    
    uint32_t raw_instr = 0;
    size_t instr_len = 0;
    
    if ((half & 0x3) != 0x3) {
      // 16비트 압축 명령어 (C Extension)
      raw_instr = static_cast<uint32_t>(half);
      instr_len = 2;
    } else {
      if (pos + sizeof(uint32_t) > text_section.size()) {
        break; // 또는 예외 처리
      }
      // 32비트 표준 명령어
      std::memcpy(&raw_instr, &text_section[pos], sizeof(uint32_t));
      instr_len = 4;
    }
      
    to_decode_list.push_back(ToDecodeInstruction{
      .pc_ = pc, 
      .bits_ = raw_instr, 
      .length_ = instr_len
    });
    
    pos += instr_len;
    pc += static_cast<uint32_t>(instr_len);
  }

  // 디스어셈블 결과 출력
  std::cout << "-----------------------------------------------------\n";
  std::cout << " PC 주소     바이너리 코드     디스어셈블 결과\n";
  std::cout << "-----------------------------------------------------\n";
  for (const auto& to_decode : to_decode_list) {
    auto decoded_instr = InstructionDecoder::decode(to_decode.bits_);
    std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex 
              << to_decode.pc_ << "    "
              << "0x" << std::setw(to_decode.length_ == 2 ? 4 : 8) 
              << std::setfill('0') << std::hex 
              << to_decode.bits_ << "    " 
              << decoded_instr->disassemble(to_decode.pc_) << "\n";
  }

  return EXIT_SUCCESS;
}
