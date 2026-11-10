#include "disasm/RV32I.h"

#include <cstdint>
#include <memory>
#include <format>


namespace {

// R-Type (rd, rs1, rs2)
class RInstruction : public Instruction {
public:
  RInstruction(uint32_t bits, std::string mne) 
    : Instruction(bits), mnemonic(std::move(mne)) {
    rd     = (bits >> 7) & 0x1F;
    rs1    = (bits >> 15) & 0x1F;
    rs2    = (bits >> 20) & 0x1F;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const
  -> std::string override {
    return std::format("{:<7} x{}, x{}, x{}", mnemonic, rd, rs1, rs2);
  }
protected:
  std::string mnemonic;
  uint32_t rd, rs1, rs2;
};

// I-Type 산술/논리 (rd, rs1, imm)
class ArithIInstruction : public Instruction {
public:
  ArithIInstruction(uint32_t bits, std::string mne) 
    : Instruction(bits), mnemonic(std::move(mne)) {
    rd  = (bits >> 7) & 0x1F;
    rs1 = (bits >> 15) & 0x1F;
    imm = static_cast<int32_t>(bits) >> 20;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const
  -> std::string override {
    return std::format("{:<7} x{}, x{}, {}", mnemonic, rd, rs1, imm);
  }
protected:
  std::string mnemonic;
  uint32_t rd, rs1;
  int32_t imm;
};

// I-Type 시프트 (rd, rs1, shamt)
class ShiftIInstruction : public Instruction {
public:
  ShiftIInstruction(uint32_t bits, std::string mne) 
    : Instruction(bits), mnemonic(std::move(mne)) {
    rd    = (bits >> 7) & 0x1F;
    rs1   = (bits >> 15) & 0x1F;
    shamt = (bits >> 20) & 0x1F;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const
  -> std::string override {
    return std::format("{:<7} x{}, x{}, {}", mnemonic, rd, rs1, shamt);
  }
protected:
  std::string mnemonic;
  uint32_t rd, rs1, shamt;
};

// I-Type Load 및 Jalr (rd, imm(rs1))
class LoadJalrInstruction : public Instruction {
public:
  LoadJalrInstruction(uint32_t bits, std::string mne) 
    : Instruction(bits), mnemonic(std::move(mne)) {
    rd  = (bits >> 7) & 0x1F;
    rs1 = (bits >> 15) & 0x1F;
    imm = static_cast<int32_t>(bits) >> 20;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const
  -> std::string override {
    return std::format("{:<7} x{}, {}(x{})", mnemonic, rd, imm, rs1);
  }
protected:
  std::string mnemonic;
  uint32_t rd, rs1;
  int32_t imm;
};

// S-Type (rs2, imm(rs1))
class SInstruction : public Instruction {
public:
  SInstruction(uint32_t bits, std::string mne) 
    : Instruction(bits), mnemonic(std::move(mne)) {
    rs1 = (bits >> 15) & 0x1F;
    rs2 = (bits >> 20) & 0x1F;
    uint32_t imm11_5 = (bits >> 25) & 0x7F;
    uint32_t imm4_0  = (bits >> 7)  & 0x1F;
    uint32_t raw_imm = (imm11_5 << 5) | imm4_0;
    imm = static_cast<int32_t>(raw_imm << 20) >> 20;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const
  -> std::string override {
    return std::format("{:<7} x{}, {}(x{})", mnemonic, rs2, imm, rs1);
  }
protected:
  std::string mnemonic;
  uint32_t rs1, rs2;
  int32_t imm;
};

// B-Type (rs1, rs2, pc + imm)
class BInstruction : public Instruction {
public:
  BInstruction(uint32_t bits, std::string mne) 
    : Instruction(bits), mnemonic(std::move(mne)) {
    rs1 = (bits >> 15) & 0x1F;
    rs2 = (bits >> 20) & 0x1F;
    uint32_t imm12   = (bits >> 31) & 0x01;
    uint32_t imm11   = (bits >> 7)  & 0x01;
    uint32_t imm10_5 = (bits >> 25) & 0x3F;
    uint32_t imm4_1  = (bits >> 8)  & 0x0F;
    uint32_t raw_imm = (imm12 << 12)
                     | (imm11 << 11)
                     | (imm10_5 << 5)
                     | (imm4_1 << 1);
    imm = static_cast<int32_t>(raw_imm << 19) >> 19;
  }
  [[nodiscard]]
  auto disassemble(uint32_t pc) const
  -> std::string override {
    auto target = static_cast<uint32_t>(static_cast<int32_t>(pc) + imm);
    return std::format("{:<7} x{}, x{}, 0x{:08x}", mnemonic, rs1, rs2, target);
  }
protected:
  std::string mnemonic;
  uint32_t rs1, rs2;
  int32_t imm;
};

// J-Type (rd, pc + imm)
class JInstruction : public Instruction {
public:
  JInstruction(uint32_t bits, std::string mne) 
    : Instruction(bits), mnemonic(std::move(mne)) {
    rd = (bits >> 7) & 0x1F;
    uint32_t bit31    = (bits >> 31) & 0x1;
    uint32_t bits30_21 = (bits >> 21) & 0x3FF;
    uint32_t bit20    = (bits >> 20) & 0x1;
    uint32_t bits19_12 = (bits >> 12) & 0xFF;
    uint32_t assembled = (bit31 << 20)
                       | (bits19_12 << 12)
                       | (bit20 << 11)
                       | (bits30_21 << 1);
    imm = static_cast<int32_t>(assembled << 11) >> 11;
  }
  [[nodiscard]]
  auto disassemble(uint32_t pc) const
  -> std::string override {
    auto target = static_cast<uint32_t>(static_cast<int32_t>(pc) + imm);
    return std::format("{:<7} x{}, 0x{:08x}", mnemonic, rd, target);
  }
protected:
  std::string mnemonic;
  uint32_t rd;
  int32_t imm;
};

// U-Type (lui: imm, auipc: pc + imm)
class UInstruction : public Instruction {
public:
  UInstruction(uint32_t bits, std::string mne) 
    : Instruction(bits), mnemonic(std::move(mne)) {
    rd = (bits >> 7) & 0x1F;
    imm = static_cast<int32_t>(bits & 0xFFFFF000);
  }
  [[nodiscard]]
  auto disassemble(uint32_t pc) const
  -> std::string override {
    uint32_t val 
      = (mnemonic == "auipc")
      ? static_cast<uint32_t>(static_cast<int32_t>(pc) + imm) 
      : static_cast<uint32_t>(imm);
    return std::format("{:<7} x{}, 0x{:08x}", mnemonic, rd, val);
  }
protected:
  std::string mnemonic;
  uint32_t rd;
  int32_t imm;
  bool is_auipc;
};

// 미정의 명령어 형식
class UnknownInstr : public Instruction {
public:
  using Instruction::Instruction;
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const
  -> std::string override {
    return "unknown";
  }
};

// Opcode 열거형
enum class Opcode : uint8_t {
  RType = 0x33,
  ITypeArith = 0x13,
  ITypeLoad = 0x03,
  ITypeJump = 0x67,
  SType = 0x23,
  BType = 0x63,
  UTypeLoad = 0x37,
  UTypeAddi = 0x17,
  JType = 0x6F
};

}

auto InstructionDecoder::decode(uint32_t bits) 
-> std::unique_ptr<Instruction> {
  auto opcode = static_cast<Opcode>(bits & 0x7F);
  switch (opcode) {
    case Opcode::RType: return decodeRType(bits);
    case Opcode::ITypeLoad: return decodeITypeLoad(bits);
    case Opcode::ITypeArith: return decodeITypeArith(bits);
    case Opcode::ITypeJump: return decodeITypeJump(bits);
    case Opcode::SType: return decodeSType(bits);
    case Opcode::BType: return decodeBType(bits);
    case Opcode::JType: return decodeJType(bits);
    case Opcode::UTypeLoad: 
    case Opcode::UTypeAddi: return decodeUType(bits);
    default: break;
  }
  return std::make_unique<UnknownInstr>(bits);
}

auto InstructionDecoder::decodeRType(uint32_t bits) 
-> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  uint32_t funct7 = (bits >> 25) & 0x7F;
  switch (funct3) {
    case 0x0:
      if (funct7 == 0x00) { return std::make_unique<RInstruction>(bits, "add"); }
      if (funct7 == 0x20) { return std::make_unique<RInstruction>(bits, "sub"); }
      break;
    case 0x1: return std::make_unique<RInstruction>(bits, "sll");
    case 0x2: return std::make_unique<RInstruction>(bits, "slt");
    case 0x3: return std::make_unique<RInstruction>(bits, "sltu");
    case 0x4: return std::make_unique<RInstruction>(bits, "xor");
    case 0x5:
      if (funct7 == 0x00) { return std::make_unique<RInstruction>(bits, "srl"); }
      if (funct7 == 0x20) { return std::make_unique<RInstruction>(bits, "sra"); }
      break;
    case 0x6: return std::make_unique<RInstruction>(bits, "or");
    case 0x7: return std::make_unique<RInstruction>(bits, "and");
    default: break;
  }
  return std::make_unique<UnknownInstr>(bits);
}

auto InstructionDecoder::decodeITypeLoad(uint32_t bits) 
-> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  switch (funct3) {
    case 0x0: return std::make_unique<LoadJalrInstruction>(bits, "lb");
    case 0x1: return std::make_unique<LoadJalrInstruction>(bits, "lh");
    case 0x2: return std::make_unique<LoadJalrInstruction>(bits, "lw");
    case 0x4: return std::make_unique<LoadJalrInstruction>(bits, "lbu");
    case 0x5: return std::make_unique<LoadJalrInstruction>(bits, "lhu");
    default: break;
  }
  return std::make_unique<UnknownInstr>(bits);
}

auto InstructionDecoder::decodeITypeArith(uint32_t bits) 
-> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  uint32_t funct7 = (bits >> 25) & 0x7F;
  switch (funct3) {
    case 0x0: return std::make_unique<ArithIInstruction>(bits, "addi");
    case 0x1: 
      if (funct7 == 0x00) { return std::make_unique<ShiftIInstruction>(bits, "slli"); }
      break;
    case 0x2: return std::make_unique<ArithIInstruction>(bits, "slti");
    case 0x3: return std::make_unique<ArithIInstruction>(bits, "sltiu");
    case 0x4: return std::make_unique<ArithIInstruction>(bits, "xori");
    case 0x5: 
      if (funct7 == 0x00) { return std::make_unique<ShiftIInstruction>(bits, "srli"); }
      if (funct7 == 0x20) { return std::make_unique<ShiftIInstruction>(bits, "srai"); }
      break;
    case 0x6: return std::make_unique<ArithIInstruction>(bits, "ori");
    case 0x7: return std::make_unique<ArithIInstruction>(bits, "andi");
    default: break;
  }
  return std::make_unique<UnknownInstr>(bits);
}

auto InstructionDecoder::decodeITypeJump(uint32_t bits) 
-> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  switch (funct3) {
    case 0x0: return std::make_unique<LoadJalrInstruction>(bits, "jalr");
    default: break;
  }
  return std::make_unique<UnknownInstr>(bits);
}

auto InstructionDecoder::decodeSType(uint32_t bits)
-> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  switch (funct3) {
    case 0x0: return std::make_unique<SInstruction>(bits, "sb");
    case 0x1: return std::make_unique<SInstruction>(bits, "sh");
    case 0x2: return std::make_unique<SInstruction>(bits, "sw");
    default: break;
  }
  return std::make_unique<UnknownInstr>(bits);
}

auto InstructionDecoder::decodeBType(uint32_t bits) 
-> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  switch (funct3) {
    case 0x0: return std::make_unique<BInstruction>(bits, "beq");
    case 0x1: return std::make_unique<BInstruction>(bits, "bne");
    case 0x4: return std::make_unique<BInstruction>(bits, "blt");
    case 0x5: return std::make_unique<BInstruction>(bits, "bge");
    case 0x6: return std::make_unique<BInstruction>(bits, "bltu");
    case 0x7: return std::make_unique<BInstruction>(bits, "bgeu");
    default: break;
  }
  return std::make_unique<UnknownInstr>(bits);
}

auto InstructionDecoder::decodeJType(uint32_t bits)
-> std::unique_ptr<Instruction> {
  return std::make_unique<JInstruction>(bits, "jal");
}

auto InstructionDecoder::decodeUType(uint32_t bits) 
-> std::unique_ptr<Instruction> {
  uint32_t opcode = bits & 0x7F;
  if (opcode == 0x37) { return std::make_unique<UInstruction>(bits, "lui"); }
  if (opcode == 0x17) { return std::make_unique<UInstruction>(bits, "auipc"); }
  return std::make_unique<UnknownInstr>(bits);
}

