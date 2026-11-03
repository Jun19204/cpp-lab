#include "disasm/ElfParser.h"
#include "disasm/RV32I.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace  {
// PC 주소와 바이너리 코드를 담은 구조체
struct DecodedInstruction {
  uint32_t pc;
  uint32_t raw_bits;
  size_t length; // 2 or 4 bytes
};
}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "usage : disasm-rv32i <file_path>\n";
    return EXIT_FAILURE;
  }

  std::string file_path(argv[1]);
  auto parser = createElfParser(file_path);
  auto text_bytes = parser->extractTextSection();

  std::vector<DecodedInstruction> decoded_list;
  size_t pos = 0;
  uint32_t pc = 0x00000000; // 시작 주소 (필요시 e_entry 기반 주소로)

  // 가변 길이 바이트 patch 및 parsing
  while (pos < text_bytes.size()) {
    if (pos + sizeof(uint16_t) > text_bytes.size()) {
      break; // 또는 예외 처리
    }
    
    uint16_t half = 0;
    std::memcpy(&half, &text_bytes[pos], sizeof(uint16_t));
    
    uint32_t raw_instr = 0;
    size_t instr_len = 0;
    
    if ((half & 0x3) != 0x3) {
      // 16비트 압축 명령어 (C Extension)
      raw_instr = static_cast<uint32_t>(half);
      instr_len = 2;
    } else {
      if (pos + sizeof(uint32_t) > text_bytes.size()) {
        break; // 또는 예외 처리
      }
      // 32비트 표준 명령어
      std::memcpy(&raw_instr, &text_bytes[pos], sizeof(uint32_t));
      instr_len = 4;
    }
      
    decoded_list.push_back(DecodedInstruction{
      .pc = pc, 
      .raw_bits = raw_instr, 
      .length = instr_len
    });
    
    pos += instr_len;
    pc += static_cast<uint32_t>(instr_len);
  }

  // 디스어셈블 결과 출력
  std::cout << "-----------------------------------------------------\n";
  std::cout << " PC 주소     바이너리 코드     디스어셈블 결과\n";
  std::cout << "-----------------------------------------------------\n";
  for (const auto& decoded : decoded_list) {
    auto instr = InstructionFactory::create(decoded.raw_bits);
    std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex 
              << decoded.pc << "    "
              << "0x" << std::setw(decoded.length == 2 ? 4 : 8) 
              << std::setfill('0') << std::hex 
              << decoded.raw_bits << "    " 
              << instr->disassemble(decoded.pc) << "\n";
  }

  return EXIT_SUCCESS;
}
