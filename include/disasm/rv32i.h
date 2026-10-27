#ifndef RV32I_H
#define RV32I_H

#include <cstdint>
#include <format>
#include <memory>
#include <string>

// 
// 최상위 추상 클래스
//
class Instruction {
protected:
  uint32_t raw_bits;
public:
  Instruction(uint32_t bits) : raw_bits(bits) {}
  virtual ~Instruction() = default;
  [[nodiscard]]
  virtual std::string disassemble([[maybe_unused]] uint32_t pc) const = 0;
};


// 
// 포멧별 중간 클래스
//
class RType : public Instruction {
protected:
  uint32_t rd, funct3, rs1, rs2, funct7;
public:
  RType(uint32_t bits) : Instruction(bits) {
    rd = (bits >> 7) & 0x1F;
    funct3 = (bits >> 12) & 0x07;
    rs1 = (bits >> 15) & 0x1F;
    rs2 = (bits >> 20) & 0x1F;
    funct7 = (bits >> 25) & 0x7F;
  }
};

class IType : public Instruction {
protected:
  uint32_t rd, funct3, rs1;
  int32_t imm;
public:
  IType(uint32_t bits) : Instruction(bits) {
    rd = (bits >> 7) & 0x1F;
    funct3 = (bits >> 12) & 0x07;
    rs1 = (bits >> 15) & 0x1F;
    imm = static_cast<int32_t>(bits) >> 20;
  }
};


class SType : public Instruction {
protected:
  uint32_t funct3, rs1, rs2;
  int32_t imm;
public:
  SType(uint32_t bits) : Instruction(bits) {
    int32_t immLow = (static_cast<int32_t>(bits) >> 7) & 0x1F;
    funct3 = (bits >> 12) & 0x07;
    rs1 = (bits >> 15) & 0x1F;
    rs2 = (bits >> 20) & 0x1F;
    int32_t immHigh = static_cast<int32_t>(bits) >> 20;
    immHigh &= static_cast<int32_t>(0xFFFFFFE0); // 하위 5비트 비워두기
    imm = immHigh | immLow;
  }
};

class BType : public Instruction {
protected:
  uint32_t funct3, rs1, rs2;
  int32_t imm;
public:
  BType(uint32_t bits) : Instruction(bits) {
    auto casted_bits = static_cast<int32_t>(bits);
    funct3 = (bits >> 12) & 0x07;
    rs1 = (bits >> 15) & 0x1F;
    rs2 = (bits >> 20) & 0x1F;
    int32_t imm_12 = casted_bits >> 19;
    imm_12 &= static_cast<int32_t>(0xFFFFF000);
    int32_t imm_11 = ((casted_bits >> 7) & 0x01) << 11;
    int32_t imm_10_5 = ((casted_bits >> 25) & 0x3F) << 5;
    int32_t imm_4_1 = ((casted_bits >> 8) & 0x0F) << 1;
    imm = imm_12 | imm_11 | imm_10_5 | imm_4_1;
  }
};



// 
// 구체적인 명령어
//
struct AddInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("add   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};

struct SubInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sub   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};


struct LwInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("lw   x{}, {}(x{})", rd, imm, rs1);
    return cmd;
  }
};

struct SwInstr : SType {
  using SType::SType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sw   x{}, {}(x{})", rs2, imm, rs1);
    return cmd;
  }
};

struct BeqInstr : BType {
  using BType::BType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("beq   x{}, x{}, pc+({})", rs1, rs2, imm);
    return cmd;
  }
};

struct UnknownInstr : Instruction {
  using Instruction::Instruction;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    return "unknown";
  }
};


//
// Opcode 열거형
//
enum class Opcode : uint8_t {
  RType = 0x33,
  ITypeArith = 0x13,
  ITypeLoad = 0x03,
  ITypeJump = 0x67,
  SType = 0x23,
  BType = 0x63,
  UType = 0x37,
  JType = 0x6F
};


// 
// 객체 생성 팩토리
//
class InstructionFactory {
public:
  // control
  static std::unique_ptr<Instruction> create(uint32_t bits) {
    auto opcode = static_cast<Opcode>(bits & 0x7F);
    switch (opcode) {
    case Opcode::RType:
        return createRType(bits);
    case Opcode::ITypeLoad:
        return createITypeLoad(bits);
    case Opcode::SType:
        return createSType(bits);
    case Opcode::BType:
        return createBType(bits);
    default: 
        break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

private:
  // ALU control
  static std::unique_ptr<Instruction> createRType(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    uint32_t funct7 = (bits >> 25) & 0x7f;
    switch (funct3) {
      case 0x0:
        if (funct7 == 0x00) { return std::make_unique<AddInstr>(bits); }
        if (funct7 == 0x20) { return std::make_unique<SubInstr>(bits); }
        break;
      default: break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createITypeLoad(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    switch (funct3) {
      case 0x2: 
        return std::make_unique<LwInstr>(bits);
      default: 
        break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createSType(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    switch (funct3) {
      case 0x2: 
        return std::make_unique<SwInstr>(bits);
      default: 
        break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createBType(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    switch (funct3) {
      case 0x0:
        return std::make_unique<BeqInstr>(bits);
      default:
        break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }
};

#endif
