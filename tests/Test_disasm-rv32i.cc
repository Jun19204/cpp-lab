#include "disasm/RV32I.h"
#include "disasm/TestParser.h"
#include "gtest/gtest.h"

#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

TEST(DisasmIntegrationTest, DumpTestBinary) {
  std::uint32_t pc = 0x00000000;
  std::string file_path = "/home/junyong/cpp-lab/resource/test.bin";

  ASSERT_TRUE(std::filesystem::exists(file_path))
      << "테스트 바이너리가 존재하지 않습니다.";

  auto instructions = Parser::parseBinary(file_path);
  ASSERT_FALSE(instructions.empty()) << "파싱한 명령어가 공백상태입니다.";

  std::cout << "\n디스어셈블리 결과 검증 테스트\n";
  std::cout << "디스어셈블 대상 파일: " << file_path << '\n';
  std::cout << "-----------------------------------------------------\n";
  std::cout << " PC 주소       바이너리 코드       디스어셈블 결과\n";
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

