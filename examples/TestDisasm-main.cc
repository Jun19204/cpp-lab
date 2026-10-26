#include "disasm/rv32i.h"

#include <cstdlib>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <vector>

int main() {
  // 테스트용 기계어 스트림 (실제 리틀 엔디안 형태의 RV32I 명령 가상 배열)
  std::vector<uint32_t> mock_binary{
      0x002081B3, // add x3, x1, x2
      0x402081B3, // sub x3, x1, x2
      0x00412183, // lw  x3, 4(x2)
      0x00312223, // sw  x3, 4(x2)
      0xFE208CE3  // beq x1, x2, pc-8  (뒤로 2개 명령어 점프 오프셋)
  };

  uint32_t pc = 0x00000000;
  std::cout << "-------------------------------------------\n";
  std::cout << " PC 주소     기계어      해석된 명령어\n";
  std::cout << "-------------------------------------------\n";

  for (uint32_t bits : mock_binary) {
    auto instr = InstructionFactory::create(bits);
    
    std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex << pc
              << "  "
              << "0x" << std::setw(8) << std::setfill('0') << std::hex << bits
              << "  " << instr->disassemble(pc) << "\n";

    pc += 4; // RISC-V는 4바이트씩 이동
  }
  return EXIT_SUCCESS;
}
