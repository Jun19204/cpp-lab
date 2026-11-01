#include "disasm/RV32I.h"
#include "disasm/ElfParser.h"

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cerr << "파일 경로를 입력하셔야 합니다.\n";
    return EXIT_FAILURE;
  }
  if (argc > 2) {
    std::cerr << "하나의 파일 경로만 입력 가능합니다.\n";
    return EXIT_FAILURE;
  }

  std::string file_path(argv[1]);
  uint32_t pc = 0x00000000;

  auto parser = createElfParser(file_path);
  auto text_bytes = parser->extractTextSection();

  std::vector<uint32_t> instructions;
  for (size_t i = 0; i + 3 < text_bytes.size(); i += 4) {
    uint32_t inst{0};
    std::memcpy(&inst, &text_bytes[i], sizeof(uint32_t));
    instructions.push_back(inst);
  }

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
    pc += 4;
  }
  return EXIT_SUCCESS;
}
