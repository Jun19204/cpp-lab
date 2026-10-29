#include "disasm/rv32i.h"
#include "disasm/parser.h"

#include <cstdlib>
#include <cstdint>
#include <iostream>

int main() {
  std::uint32_t pc = 0x00000000;

  auto instructions = Parser::parseBinary("/home/junyong/cpp-lab/resource/TestBinary.o");

  std::cout << "-----------------------------------------------------\n";
  std::cout << " PC 주소     바이너리 코드     디스어셈블 결과\n";
  std::cout << "-----------------------------------------------------\n";

  for (std::uint32_t bits : instructions) {
    auto instr = InstructionFactory::create(bits);
    
    std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex << pc
              << "    "
              << "0x" << std::setw(8) << std::setfill('0') << std::hex << bits
              << "    " 
              << instr->disassemble(pc) << "\n";
    
    pc += 4; // 4바이트씩 이동
  }
  return EXIT_SUCCESS;
}
