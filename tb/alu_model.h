// alu_model.h: a C++ reference model of rtl/alu.sv.
//
// The testbench runs the same inputs through this model and through the
// hardware and checks that they agree. It is written the "obvious" software
// way so it is easy to trust.
#pragma once

#include <cstdint>

// Must match the numbers in rtl/alu_pkg.sv.
enum AluOp : uint8_t {
  ALU_ADD  = 0,
  ALU_SUB  = 1,
  ALU_AND  = 2,
  ALU_OR   = 3,
  ALU_XOR  = 4,
  ALU_SLL  = 5,
  ALU_SRL  = 6,
  ALU_SRA  = 7,
  ALU_SLT  = 8,
  ALU_SLTU = 9,
  ALU_NUM_OPS = 10
};

inline const char* alu_op_name(int op) {
  static const char* names[ALU_NUM_OPS] = {
    "add", "sub", "and", "or", "xor", "sll", "srl", "sra", "slt", "sltu"};
  return (op >= 0 && op < ALU_NUM_OPS) ? names[op] : "???";
}

inline uint32_t alu_model(int op, uint32_t a, uint32_t b) {
  uint32_t shamt = b & 0x1F;  // only the low 5 bits of b
  switch (op) {
    case ALU_ADD:  return a + b;  // uint32_t wraps on overflow, like hardware
    case ALU_SUB:  return a - b;
    case ALU_AND:  return a & b;
    case ALU_OR:   return a | b;
    case ALU_XOR:  return a ^ b;
    case ALU_SLL:  return a << shamt;
    case ALU_SRL:  return a >> shamt;
    case ALU_SRA:  return (uint32_t)((int32_t)a >> shamt);  // arithmetic shift on signed
    case ALU_SLT:  return (int32_t)a < (int32_t)b ? 1 : 0;
    case ALU_SLTU: return a < b ? 1 : 0;
    default:       return 0;
  }
}
