#ifndef RV32I_H
#define RV32I_H

#include <cstdint>
#include <format>
#include <iomanip>
#include <ios>
#include <memory>
#include <sstream>
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
// 포멧별 Intermediate 클래스
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
    funct3 = (bits >> 12) & 0x07;
    rs1    = (bits >> 15) & 0x1F;
    rs2    = (bits >> 20) & 0x1F;
    uint32_t imm11_5 = (bits >> 25) & 0x7F; // 상위 7비트
    uint32_t imm4_0  = (bits >> 7)  & 0x1F; // 하위 5비트
    uint32_t raw_imm = (imm11_5 << 5) | imm4_0;
    imm = static_cast<int32_t>(raw_imm << 20) >> 20;
  }
};

class BType : public Instruction {
protected:
  uint32_t funct3, rs1, rs2;
  int32_t imm;
public:
  BType(uint32_t bits) : Instruction(bits) {
    funct3 = (bits >> 12) & 0x07;
    rs1    = (bits >> 15) & 0x1F;
    rs2    = (bits >> 20) & 0x1F;
    uint32_t imm12   = (bits >> 31) & 0x01;
    uint32_t imm11   = (bits >> 7)  & 0x01;
    uint32_t imm10_5 = (bits >> 25) & 0x3F;
    uint32_t imm4_1  = (bits >> 8)  & 0x0F;
    uint32_t raw_imm = (imm12  << 12) 
                    | (imm11  << 11) 
                    | (imm10_5 << 5) 
                    | (imm4_1  << 1);
    imm = static_cast<int32_t>(raw_imm << 19) >> 19;
  }
};

class JType : public Instruction {
protected:
  uint32_t rd;
  int32_t imm;
public:
  JType(uint32_t bits) : Instruction(bits) {
    rd = (bits >> 7) & 0x1F;
    uint32_t bit31    = (bits >> 31) & 0x1;
    uint32_t bits30_21 = (bits >> 21) & 0x3FF;
    uint32_t bit20    = (bits >> 20) & 0x1;
    uint32_t bits19_12 = (bits >> 12) & 0xFF;
    uint32_t assembled = (bit31    << 20)
                       | (bits19_12 << 12)
                       | (bit20     << 11)
                       | (bits30_21 << 1);
    imm = static_cast<int32_t>(assembled << 11) >> 11;
  }
};

class UType : public Instruction {
protected:
  uint32_t rd;
  int32_t imm;
public:
  UType(uint32_t bits) : Instruction(bits) {
    rd = (bits >> 7) & 0x1F;
    imm = static_cast<int32_t>(bits & 0xFFFFF000);
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
struct SllInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sll   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};
struct SltInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("slt   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};
struct SltuInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sltu   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};
struct SrlInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("srl   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};
struct SraInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sra   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};
struct XorInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("xor   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};
struct OrInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("or   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};
struct AndInstr : RType {
  using RType::RType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("and   x{}, x{}, x{}", rd, rs1, rs2);
    return cmd;
  }
};

struct AddiInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("addi   x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct SlliInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("slli    x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct SltiInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("slti    x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct SltiuInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sltiu    x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct XoriInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("xori    x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct SraiInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("srai    x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct SrliInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("srli    x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct OriInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("ori    x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct AndiInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("andi    x{}, x{}, {}", rd, rs1, imm);
    return cmd;
  }
};
struct LwInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("lw    x{}, {}(x{})", rd, imm, rs1);
    return cmd;
  }
};
struct LbInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("lb    x{}, {}(x{})", rd, imm, rs1);
    return cmd;
  }
};
struct LhInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("lh    x{}, {}(x{})", rd, imm, rs1);
    return cmd;
  }
};
struct LbuInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("lbu    x{}, {}(x{})", rd, imm, rs1);
    return cmd;
  }
};
struct LhuInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("lhu    x{}, {}(x{})", rd, imm, rs1);
    return cmd;
  }
};
struct JalrInstr : IType {
  using IType::IType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("jalr   x{}, {}(x{})", rd, imm, rs1);
    return cmd;
  }
};

struct SwInstr : SType {
  using SType::SType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sw    x{}, {}(x{})", rs2, imm, rs1);
    return cmd;
  }
};
struct SbInstr : SType {
  using SType::SType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sb    x{}, {}(x{})", rs2, imm, rs1);
    return cmd;
  }
};
struct ShInstr : SType {
  using SType::SType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::string cmd = std::format("sh    x{}, {}(x{})", rs2, imm, rs1);
    return cmd;
  }
};

struct BeqInstr : BType {
  using BType::BType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream target_address;
    target_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << static_cast<int32_t>(pc) + imm;
    std::string cmd 
      = std::format("beq   x{}, x{}, {}", rs1, rs2, target_address.str());
    return cmd;
  }
};
struct BneInstr : BType {
  using BType::BType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream target_address;
    target_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << static_cast<int32_t>(pc) + imm;
    std::string cmd 
      = std::format("bne   x{}, x{}, {}", rs1, rs2, target_address.str());
    return cmd;
  }
};
struct BltInstr : BType {
  using BType::BType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream target_address;
    target_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << static_cast<int32_t>(pc) + imm;
    std::string cmd 
      = std::format("blt   x{}, x{}, {}", rs1, rs2, target_address.str());
    return cmd;
  }
};
struct BgeInstr : BType {
  using BType::BType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream target_address;
    target_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << static_cast<int32_t>(pc) + imm;
    std::string cmd 
      = std::format("bge   x{}, x{}, {}", rs1, rs2, target_address.str());
    return cmd;
  }
};
struct BltuInstr : BType {
  using BType::BType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream target_address;
    target_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << static_cast<int32_t>(pc) + imm;
    std::string cmd 
      = std::format("bltu   x{}, x{}, {}", rs1, rs2, target_address.str());
    return cmd;
  }
};
struct BgeuInstr : BType {
  using BType::BType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream target_address;
    target_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << static_cast<int32_t>(pc) + imm;
    std::string cmd 
      = std::format("bgeu   x{}, x{}, {}", rs1, rs2, target_address.str());
    return cmd;
  }
};

struct JalInstr : JType {
  using JType::JType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream target_address;
    target_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << static_cast<int32_t>(pc) + imm;
    std::string cmd 
      = std::format("jal   x{}, {}", rd, target_address.str());
    return cmd;
  }
};

struct LuiInstr : UType {
  using UType::UType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream extended_address;
    extended_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << imm;
    std::string cmd 
      = std::format("lui   x{}, {}", rd, extended_address.str());
    return cmd;
  }
};
struct AuipcInstr : UType {
  using UType::UType;
  [[nodiscard]]
  std::string disassemble([[maybe_unused]] uint32_t pc) const override {
    std::stringstream target_address;
    target_address << "0x" << std::setw(8) << std::setfill('0') 
                   << std::hex << static_cast<int32_t>(pc) + imm;
    std::string cmd 
      = std::format("auipc   x{}, {}", rd, target_address.str());
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
  UTypeLoad = 0x37,
  UTypeAddi = 0x17,
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
      case Opcode::RType: return createRType(bits);
      case Opcode::ITypeLoad: return createITypeLoad(bits);
      case Opcode::ITypeArith: return createITypeArith(bits);
      case Opcode::ITypeJump: return createITypeJump(bits);
      case Opcode::SType: return createSType(bits);
      case Opcode::BType: return createBType(bits);
      case Opcode::JType: return createJType(bits);
      case Opcode::UTypeLoad: 
      case Opcode::UTypeAddi: return createUType(bits);
      default: break;
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
      case 0x1: return std::make_unique<SllInstr>(bits);
      case 0x2: return std::make_unique<SltInstr>(bits);
      case 0x3: return std::make_unique<SltuInstr>(bits);
      case 0x4: return std::make_unique<XorInstr>(bits);
      case 0x5:
        if (funct7 == 0x00) { return std::make_unique<SrlInstr>(bits); }
        if (funct7 == 0x20) { return std::make_unique<SraInstr>(bits); }
        break;
      case 0x6: return std::make_unique<OrInstr>(bits);
      case 0x7: return std::make_unique<AndInstr>(bits);
      default: break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createITypeLoad(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    switch (funct3) {
      case 0x0: return std::make_unique<LbInstr>(bits);
      case 0x1: return std::make_unique<LhInstr>(bits);
      case 0x2: return std::make_unique<LwInstr>(bits);
      case 0x4: return std::make_unique<LbuInstr>(bits);
      case 0x5: return std::make_unique<LhuInstr>(bits);
      default: break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createITypeArith(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    uint32_t funct7 = (bits >> 25) & 0x7f;
    switch (funct3) {
      case 0x0: return std::make_unique<AddiInstr>(bits);
      case 0x1: return std::make_unique<SlliInstr>(bits);
      case 0x2: return std::make_unique<SltiInstr>(bits);
      case 0x3: return std::make_unique<SltiuInstr>(bits);
      case 0x4: return std::make_unique<XoriInstr>(bits);
      case 0x5: 
        if (funct7 == 0x20) { return std::make_unique<SraiInstr>(bits); }
        if (funct7 == 0x00) { return std::make_unique<SrliInstr>(bits); }
        break;
      case 0x6: return std::make_unique<OriInstr>(bits);
      case 0x7: return std::make_unique<AndiInstr>(bits);
      default: break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createITypeJump(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    switch (funct3) {
      case 0x0: return std::make_unique<JalrInstr>(bits);
      default: break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createSType(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    switch (funct3) {
      case 0x0: return std::make_unique<SbInstr>(bits);
      case 0x1: return std::make_unique<ShInstr>(bits);
      case 0x2: return std::make_unique<SwInstr>(bits);
      default: break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createBType(uint32_t bits) {
    uint32_t funct3 = (bits >> 12) & 0x07;
    switch (funct3) {
      case 0x0: return std::make_unique<BeqInstr>(bits);
      case 0x1: return std::make_unique<BneInstr>(bits);
      case 0x4: return std::make_unique<BltInstr>(bits);
      case 0x5: return std::make_unique<BgeInstr>(bits);
      case 0x6: return std::make_unique<BltuInstr>(bits);
      case 0x7: return std::make_unique<BgeuInstr>(bits);
      default: break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }

  static std::unique_ptr<Instruction> createJType(uint32_t bits) {
    return std::make_unique<JalInstr>(bits);
  }

  static std::unique_ptr<Instruction> createUType(uint32_t bits) {
    uint32_t opcode = bits & 0x7f;
    switch (opcode) {
      case 0x37: return std::make_unique<LuiInstr>(bits);
      case 0x17: return std::make_unique<AuipcInstr>(bits);
      default: break;
    }
    return std::make_unique<UnknownInstr>(bits);
  }
};

#endif
