#ifndef RV32I_H
#define RV32I_H

#include <cstdint>
#include <string>
#include <memory>

// 인스트럭션 최상위 추상 클래스
class Instruction {
protected:
  uint32_t raw_bits;
public:
  Instruction(uint32_t bits) : raw_bits(bits) {}
  virtual ~Instruction() = default;
  [[nodiscard]]
  virtual std::string disassemble([[maybe_unused]] uint32_t pc) const = 0;
};

// 형식별 인스트럭션 객체 생성 팩토리
class InstructionDecoder {
public:
  static auto decode(uint32_t bits) -> std::unique_ptr<Instruction>;
private:
  static auto decodeRType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeITypeLoad(uint32_t bits) -> std::unique_ptr<Instruction>; 
  static auto decodeITypeArith(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeITypeJump(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeSType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeBType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeJType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeUType(uint32_t bits) -> std::unique_ptr<Instruction>;
 };


#endif
