
@startuml RV32I_Disassembler

title RV32I Disassembler - UML Class Diagram

' ============================================================
' General configuration
' ============================================================

top to bottom direction

skinparam shadowing false
skinparam backgroundColor white
skinparam classAttributeIconSize 0
skinparam linetype ortho
skinparam packageStyle rectangle
skinparam roundCorner 8

skinparam class {
  BackgroundColor #FFFFFF
  BorderColor #374151
  ArrowColor #374151
  FontColor #111827
}

skinparam package {
  BackgroundColor #F9FAFB
  BorderColor #9CA3AF
}

hide empty members
hide circle

' ============================================================
' Public API - RV32I.h
' ============================================================

package "RV32I.h (Public API)" {

  abstract class Instruction {
    # raw_bits : uint32_t
    --
    + Instruction(bits : uint32_t)
    + {abstract} disassemble(pc : uint32_t) : std::string
    + {virtual} ~Instruction()
  }

  class InstructionFactory <<Factory>> {
    + {static} create(bits : uint32_t) : std::unique_ptr<Instruction>
    --
    - {static} createRType(bits : uint32_t) : std::unique_ptr<Instruction>
    - {static} createITypeLoad(bits : uint32_t) : std::unique_ptr<Instruction>
    - {static} createITypeArith(bits : uint32_t) : std::unique_ptr<Instruction>
    - {static} createITypeJump(bits : uint32_t) : std::unique_ptr<Instruction>
    - {static} createSType(bits : uint32_t) : std::unique_ptr<Instruction>
    - {static} createBType(bits : uint32_t) : std::unique_ptr<Instruction>
    - {static} createJType(bits : uint32_t) : std::unique_ptr<Instruction>
    - {static} createUType(bits : uint32_t) : std::unique_ptr<Instruction>
  }
}

' ============================================================
' Internal implementation - RV32I.cc
' ============================================================

package "RV32I.cc (Anonymous Namespace)" {

  class RInstruction {
    # mnemonic : std::string
    # rd : uint32_t
    # rs1 : uint32_t
    # rs2 : uint32_t
    --
    + RInstruction(bits : uint32_t, mne : std::string)
    + disassemble(pc : uint32_t) : std::string
  }

  class ArithIInstruction {
    # mnemonic : std::string
    # rd : uint32_t
    # rs1 : uint32_t
    # imm : int32_t
    --
    + ArithIInstruction(bits : uint32_t, mne : std::string)
    + disassemble(pc : uint32_t) : std::string
  }

  class ShiftIInstruction {
    # mnemonic : std::string
    # rd : uint32_t
    # rs1 : uint32_t
    # shamt : uint32_t
    --
    + ShiftIInstruction(bits : uint32_t, mne : std::string)
    + disassemble(pc : uint32_t) : std::string
  }

  class LoadJalrInstruction {
    # mnemonic : std::string
    # rd : uint32_t
    # rs1 : uint32_t
    # imm : int32_t
    --
    + LoadJalrInstruction(bits : uint32_t, mne : std::string)
    + disassemble(pc : uint32_t) : std::string
  }

  class SInstruction {
    # mnemonic : std::string
    # rs1 : uint32_t
    # rs2 : uint32_t
    # imm : int32_t
    --
    + SInstruction(bits : uint32_t, mne : std::string)
    + disassemble(pc : uint32_t) : std::string
  }

  class BInstruction {
    # mnemonic : std::string
    # rs1 : uint32_t
    # rs2 : uint32_t
    # imm : int32_t
    --
    + BInstruction(bits : uint32_t, mne : std::string)
    + disassemble(pc : uint32_t) : std::string
  }

  class JInstruction {
    # mnemonic : std::string
    # rd : uint32_t
    # imm : int32_t
    --
    + JInstruction(bits : uint32_t, mne : std::string)
    + disassemble(pc : uint32_t) : std::string
  }

  class UInstruction {
    # mnemonic : std::string
    # rd : uint32_t
    # imm : int32_t
    # is_auipc : bool
    --
    + UInstruction(bits : uint32_t, mne : std::string, auipc : bool)
    + disassemble(pc : uint32_t) : std::string
  }

  class UnknownInstr {
    + disassemble(pc : uint32_t) : std::string
  }

  enum Opcode {
    RType
    ITypeArith
    ITypeLoad
    ITypeJump
    SType
    BType
    UTypeLoad
    UTypeAddi
    JType
  }
}

' ============================================================
' Parser - TestParser.h
' ============================================================

package "TestParser.h" {
  class "Parser::parseBinary" as ParseBinary <<Utility Function>> {
    + {static} parseBinary(filepath : std::filesystem::path) : std::vector<uint32_t>
  }
}

' ============================================================
' Integration test
' ============================================================

package "Test_disasm-rv32i.cc" {
  class "DisasmIntegrationTest" as IntegrationTest <<GoogleTest>> {
    + DumpTestBinary()
  }
}

' ============================================================
' Generalization - Inheritance
' ============================================================

Instruction <|-- RInstruction
Instruction <|-- ArithIInstruction
Instruction <|-- ShiftIInstruction
Instruction <|-- LoadJalrInstruction
Instruction <|-- SInstruction
Instruction <|-- BInstruction
Instruction <|-- JInstruction
Instruction <|-- UInstruction
Instruction <|-- UnknownInstr

' ============================================================
' Dependencies
' ============================================================

InstructionFactory ..> Instruction : returns unique_ptr

InstructionFactory ..> RInstruction : creates
InstructionFactory ..> ArithIInstruction : creates
InstructionFactory ..> ShiftIInstruction : creates
InstructionFactory ..> LoadJalrInstruction : creates
InstructionFactory ..> SInstruction : creates
InstructionFactory ..> BInstruction : creates
InstructionFactory ..> JInstruction : creates
InstructionFactory ..> UInstruction : creates
InstructionFactory ..> UnknownInstr : creates

InstructionFactory ..> Opcode : uses

IntegrationTest ..> ParseBinary : calls
IntegrationTest ..> InstructionFactory : calls
IntegrationTest ..> Instruction : disassemble()

' ============================================================
' Notes
' ============================================================

note right of Instruction
  Abstract base class

  Defines the polymorphic
  disassembly interface.

  raw_bits stores the
  original 32-bit encoding.
end note

note right of InstructionFactory
  Static Factory

  Decodes opcode, funct3,
  and funct7 to construct
  the corresponding object.

  Ownership is transferred
  via std::unique_ptr.
end note

note bottom of ParseBinary
  Namespace-level inline function,
  represented here as a utility
  for diagram readability.

  Reads binary instructions
  and returns uint32_t values.
end note

@enduml
