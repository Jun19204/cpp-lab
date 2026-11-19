#ifndef RV32I_H
#define RV32I_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>


// 인스트럭션 최상위 추상 클래스
class Instruction {
public:
  Instruction(uint32_t bits, std::string mne = "unknown") 
    : bits_(bits), mnemonic_(std::move(mne)) {}
  virtual ~Instruction() = default;
  [[nodiscard]]
  virtual auto disassemble([[maybe_unused]] uint32_t pc) const 
    -> std::string = 0;
protected:
  const uint32_t bits_;
  const std::string mnemonic_;
};


// 인스트럭션 포맷별 디코드 함수 팩토리
class InstructionDecoder {
public:
  [[nodiscard]]
  static auto decode(uint32_t bits) -> std::unique_ptr<Instruction>;
private:
  static auto decodeRType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeITypeLoad(uint32_t bits) -> std::unique_ptr<Instruction>; 
  static auto decodeITypeArithLogicShift(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeITypeJump(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeSType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeBType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeJType(uint32_t bits) -> std::unique_ptr<Instruction>;
  static auto decodeUType(uint32_t bits) -> std::unique_ptr<Instruction>;
};


#endif
