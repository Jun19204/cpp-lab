#include "decoder/rv32i.h"

#include <string>
#include <cstdint>
#include <memory>
#include <format>
#include <utility>


namespace {

// R-Type
class RType final : public Instruction {
public:
  RType(uint32_t bits, std::string mne) 
    : Instruction(bits, std::move(mne)) {
    rd_     = (bits >> 7) & 0x1F;
    rs1_    = (bits >> 15) & 0x1F;
    rs2_    = (bits >> 20) & 0x1F;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const 
    -> std::string override {
    return std::format("{:<7} x{}, x{}, x{}", 
                       mnemonic_, rd_, rs1_, rs2_);
  }
protected:
  uint32_t rd_, rs1_, rs2_;
};

// I-Type 산술/논리
class ITypeArithLogic : public Instruction {
public:
  ITypeArithLogic(uint32_t bits, std::string mne) 
    : Instruction(bits, std::move(mne)) {
    rd_  = (bits >> 7) & 0x1F;
    rs1_ = (bits >> 15) & 0x1F;
    imm_ = static_cast<int32_t>(bits) >> 20;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const
    -> std::string override {
    return std::format("{:<7} x{}, x{}, {}", 
                       mnemonic_, rd_, rs1_, imm_);
  }
protected:
  uint32_t rd_, rs1_;
  int32_t imm_;
};

// I-Type Load/Jalr
class ITypeLoadJalr final : public ITypeArithLogic {
public:
  using ITypeArithLogic::ITypeArithLogic;
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const 
    -> std::string override {
    return std::format("{:<7} x{}, {}(x{})", 
                       mnemonic_, rd_, imm_, rs1_);
  }
};

// I-Type 시프트
class ITypeShift final : public Instruction {
public:
  ITypeShift(uint32_t bits, std::string mne) 
    : Instruction(bits, std::move(mne)) {
    rd_    = (bits >> 7) & 0x1F;
    rs1_   = (bits >> 15) & 0x1F;
    shamt_ = (bits >> 20) & 0x1F;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const 
    -> std::string override {
    return std::format("{:<7} x{}, x{}, {}", 
                       mnemonic_, rd_, rs1_, shamt_);
  }
protected:
  uint32_t rd_, rs1_, shamt_;
};

// S-Type
class SType final : public Instruction {
public:
  SType(uint32_t bits, std::string mne) 
    : Instruction(bits, std::move(mne)) {
    rs1_ = (bits >> 15) & 0x1F;
    rs2_ = (bits >> 20) & 0x1F;
    uint32_t imm11_5 = (bits >> 25) & 0x7F;
    uint32_t imm4_0  = (bits >> 7)  & 0x1F;
    uint32_t assembled_imm = (imm11_5 << 5) | imm4_0;
    imm_ = static_cast<int32_t>(assembled_imm << 20) >> 20;
  }
  [[nodiscard]]
  auto disassemble([[maybe_unused]] uint32_t pc) const 
    -> std::string override {
    return std::format("{:<7} x{}, {}(x{})", 
                       mnemonic_, rs2_, imm_, rs1_);
  }
protected:
  uint32_t rs1_, rs2_;
  int32_t imm_;
};

// B-Type
class BType final : public Instruction {
public:
  BType(uint32_t bits, std::string mne) 
    : Instruction(bits, std::move(mne)) {
    rs1_ = (bits >> 15) & 0x1F;
    rs2_ = (bits >> 20) & 0x1F;
    uint32_t imm12   = (bits >> 31) & 0x01;
    uint32_t imm11   = (bits >> 7)  & 0x01;
    uint32_t imm10_5 = (bits >> 25) & 0x3F;
    uint32_t imm4_1  = (bits >> 8)  & 0x0F;
    uint32_t assembled_imm = (imm12 << 12)
                           | (imm11 << 11)
                           | (imm10_5 << 5)
                           | (imm4_1 << 1);
    imm_ = static_cast<int32_t>(assembled_imm << 19) >> 19;
  }
  [[nodiscard]]
  auto disassemble(uint32_t pc) const 
    -> std::string override {
    auto target = pc + static_cast<uint32_t>(imm_);
    return std::format("{:<7} x{}, x{}, 0x{:08x}", 
                       mnemonic_, rs1_, rs2_, target);
  }
protected:
  uint32_t rs1_, rs2_;
  int32_t imm_;
};

// J-Type
class JType final : public Instruction {
public:
  JType(uint32_t bits, std::string mne) 
    : Instruction(bits, std::move(mne)) {
    rd_ = (bits >> 7) & 0x1F;
    uint32_t imm_31    = (bits >> 31) & 0x1;
    uint32_t imm_30_21 = (bits >> 21) & 0x3FF;
    uint32_t imm_20    = (bits >> 20) & 0x1;
    uint32_t imm_19_12 = (bits >> 12) & 0xFF;
    uint32_t assembled_imm = (imm_31 << 20)
                           | (imm_19_12 << 12)
                           | (imm_20 << 11)
                           | (imm_30_21 << 1);
    imm_ = static_cast<int32_t>(assembled_imm << 11) >> 11;
  }
  [[nodiscard]]
  auto disassemble(uint32_t pc) const
    -> std::string override {
    auto target = pc + static_cast<uint32_t>(imm_);
    return std::format("{:<7} x{}, 0x{:08x}", 
                       mnemonic_, rd_, target);
  }
protected:
  uint32_t rd_;
  int32_t imm_;
};

// U-Type
class UType final : public Instruction {
public:
  UType(uint32_t bits, std::string mne) 
    : Instruction(bits, std::move(mne)) {
    rd_ = (bits >> 7) & 0x1F;
    imm_ = static_cast<int32_t>(bits & 0xFFFFF000);
  }
  [[nodiscard]]
  auto disassemble(uint32_t pc) const 
    -> std::string override {
    uint32_t val = (mnemonic_ == "auipc")
                 ? pc + static_cast<uint32_t>(imm_) 
                 : static_cast<uint32_t>(imm_);
    return std::format("{:<7} x{}, 0x{:08x}", 
                       mnemonic_, rd_, val);
  }
protected:
  uint32_t rd_;
  int32_t imm_;
};

// 미정의 인스트럭션 포맷
class Unknown final : public Instruction {
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
  ITypeArithLogicShift = 0x13,
  ITypeLoad = 0x03,
  ITypeJump = 0x67,
  SType = 0x23,
  BType = 0x63,
  UTypeLoad = 0x37,
  UTypeAddi = 0x17,
  JType = 0x6F
};

} // namespace


auto InstructionDecoder::decode(uint32_t bits) 
  -> std::unique_ptr<Instruction> {
  auto opcode = static_cast<Opcode>(bits & 0x7F);
  switch (opcode) {
    case Opcode::RType: return decodeRType(bits);
    case Opcode::ITypeLoad: return decodeITypeLoad(bits);
    case Opcode::ITypeArithLogicShift: return decodeITypeArithLogicShift(bits);
    case Opcode::ITypeJump: return decodeITypeJump(bits);
    case Opcode::SType: return decodeSType(bits);
    case Opcode::BType: return decodeBType(bits);
    case Opcode::JType: return decodeJType(bits);
    case Opcode::UTypeLoad: 
    case Opcode::UTypeAddi: return decodeUType(bits);
    default: break;
  }
  return std::make_unique<Unknown>(bits);
}

auto InstructionDecoder::decodeRType(uint32_t bits) 
  -> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  uint32_t funct7 = (bits >> 25) & 0x7F;
  switch (funct3) {
    case 0x0:
      if (funct7 == 0x00) { return std::make_unique<RType>(bits, "add"); }
      if (funct7 == 0x20) { return std::make_unique<RType>(bits, "sub"); }
      break;
    case 0x1: return std::make_unique<RType>(bits, "sll");
    case 0x2: return std::make_unique<RType>(bits, "slt");
    case 0x3: return std::make_unique<RType>(bits, "sltu");
    case 0x4: return std::make_unique<RType>(bits, "xor");
    case 0x5:
      if (funct7 == 0x00) { return std::make_unique<RType>(bits, "srl"); }
      if (funct7 == 0x20) { return std::make_unique<RType>(bits, "sra"); }
      break;
    case 0x6: return std::make_unique<RType>(bits, "or");
    case 0x7: return std::make_unique<RType>(bits, "and");
    default: break;
  }
  return std::make_unique<Unknown>(bits);
}

auto InstructionDecoder::decodeITypeLoad(uint32_t bits) 
  -> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  switch (funct3) {
    case 0x0: return std::make_unique<ITypeLoadJalr>(bits, "lb");
    case 0x1: return std::make_unique<ITypeLoadJalr>(bits, "lh");
    case 0x2: return std::make_unique<ITypeLoadJalr>(bits, "lw");
    case 0x4: return std::make_unique<ITypeLoadJalr>(bits, "lbu");
    case 0x5: return std::make_unique<ITypeLoadJalr>(bits, "lhu");
    default: break;
  }
  return std::make_unique<Unknown>(bits);
}

auto InstructionDecoder::decodeITypeArithLogicShift(uint32_t bits) 
  -> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  uint32_t funct7 = (bits >> 25) & 0x7F;
  switch (funct3) {
    case 0x0: return std::make_unique<ITypeArithLogic>(bits, "addi");
    case 0x1: 
      if (funct7 == 0x00) { return std::make_unique<ITypeShift>(bits, "slli"); }
      break;
    case 0x2: return std::make_unique<ITypeArithLogic>(bits, "slti");
    case 0x3: return std::make_unique<ITypeArithLogic>(bits, "sltiu");
    case 0x4: return std::make_unique<ITypeArithLogic>(bits, "xori");
    case 0x5: 
      if (funct7 == 0x00) { return std::make_unique<ITypeShift>(bits, "srli"); }
      if (funct7 == 0x20) { return std::make_unique<ITypeShift>(bits, "srai"); }
      break;
    case 0x6: return std::make_unique<ITypeArithLogic>(bits, "ori");
    case 0x7: return std::make_unique<ITypeArithLogic>(bits, "andi");
    default: break;
  }
  return std::make_unique<Unknown>(bits);
}

auto InstructionDecoder::decodeITypeJump(uint32_t bits) 
  -> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  switch (funct3) {
    case 0x0: return std::make_unique<ITypeLoadJalr>(bits, "jalr");
    default: break;
  }
  return std::make_unique<Unknown>(bits);
}

auto InstructionDecoder::decodeSType(uint32_t bits)
  -> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  switch (funct3) {
    case 0x0: return std::make_unique<SType>(bits, "sb");
    case 0x1: return std::make_unique<SType>(bits, "sh");
    case 0x2: return std::make_unique<SType>(bits, "sw");
    default: break;
  }
  return std::make_unique<Unknown>(bits);
}

auto InstructionDecoder::decodeBType(uint32_t bits) 
  -> std::unique_ptr<Instruction> {
  uint32_t funct3 = (bits >> 12) & 0x07;
  switch (funct3) {
    case 0x0: return std::make_unique<BType>(bits, "beq");
    case 0x1: return std::make_unique<BType>(bits, "bne");
    case 0x4: return std::make_unique<BType>(bits, "blt");
    case 0x5: return std::make_unique<BType>(bits, "bge");
    case 0x6: return std::make_unique<BType>(bits, "bltu");
    case 0x7: return std::make_unique<BType>(bits, "bgeu");
    default: break;
  }
  return std::make_unique<Unknown>(bits);
}

auto InstructionDecoder::decodeJType(uint32_t bits)
  -> std::unique_ptr<Instruction> {
  return std::make_unique<JType>(bits, "jal");
}

auto InstructionDecoder::decodeUType(uint32_t bits) 
  -> std::unique_ptr<Instruction> {
  uint32_t opcode = bits & 0x7F;
  if (opcode == 0x37) { return std::make_unique<UType>(bits, "lui"); }
  if (opcode == 0x17) { return std::make_unique<UType>(bits, "auipc"); }
  return std::make_unique<Unknown>(bits);
}
