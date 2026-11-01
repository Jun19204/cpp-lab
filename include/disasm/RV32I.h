#ifndef RV32I_H
#define RV32I_H

#include <cstdint>
#include <string>
#include <memory>

// 최상위 추상 클래스
class Instruction {
protected:
  uint32_t raw_bits;
public:
  Instruction(uint32_t bits) : raw_bits(bits) {}
  virtual ~Instruction() = default;
  [[nodiscard]]
  virtual std::string disassemble([[maybe_unused]] uint32_t pc) const = 0;
};

// 객체 생성 팩토리 (문자열만 주입)
class InstructionFactory {
public:
  static auto create(uint32_t bits) -> std::unique_ptr<Instruction>;
private:
  static auto createRType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto createITypeLoad(uint32_t bits) -> std::unique_ptr<Instruction>; 
  static auto createITypeArith(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto createITypeJump(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto createSType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto createBType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto createJType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto createUType(uint32_t bits) -> std::unique_ptr<Instruction>;
 };

#endif
