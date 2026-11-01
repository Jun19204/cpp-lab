#include "disasm/TestParser.h"
#include "disasm/rv32i.h"
#include "gtest/gtest.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

TEST(DisasmIntegrationTest, DumpTestBinary) {
  std::uint32_t pc = 0x00000000;
  std::string file_path("/home/junyong/cpp-lab/resource/test.bin");

  // 파일이 정상적으로 읽히는지 검증
  ASSERT_TRUE(std::filesystem::exists(file_path))
      << "Test binary file does not exist!";
  auto text_bytes = Parser::parseBinary(file_path);
  ASSERT_FALSE(text_bytes.empty()) << "Parsed binary byte stream is empty!";

  std::vector<uint32_t> instructions;
  for (size_t i = 0; i + 3 < text_bytes.size(); i += 4) {
    uint32_t inst{0};
    std::memcpy(&inst, &text_bytes[i], sizeof(uint32_t));
    instructions.push_back(inst);
  }

  std::cout << "\n[Disassembly Output for Verification]\n";
  std::cout << "-----------------------------------------------------\n";
  for (std::uint32_t bits : instructions) {
    auto instr = InstructionFactory::create(bits);
    std::cout << "0x" << std::setw(8) << std::setfill('0') << std::hex << pc
              << "    "
              << "0x" << std::setw(8) << std::setfill('0') << std::hex << bits
              << "    " << instr->disassemble(pc) << "\n";
    pc += 4;
  }

  SUCCEED();
}
