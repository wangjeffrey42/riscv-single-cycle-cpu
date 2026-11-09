// imm_gen_tb.cpp: tests rtl/imm_gen.sv with real RISC-V instruction encodings.
//
// 1. Hand-checked instructions (e.g. addi x1, x0, -5).
// 2. The largest and smallest 12-bit immediates (2047 and -2048).
// 3. 1,000 random I-type instructions checked against a C++ model.
//
// Writes build/imm_gen.vcd.

#include <cstdio>
#include <cstdlib>
#include <random>

#include "Vimm_gen.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

static VerilatedContext ctx;
static Vimm_gen* dut;
static VerilatedVcdC* wave;
static int passed = 0, total = 0;

// I-type opcodes (must match rtl/imm_gen.sv).
static const uint32_t I_TYPE_OPCODES[] = {0x13 /* OP-IMM */, 0x03 /* LOAD */, 0x67 /* JALR */};

// Reference model: arithmetic shift right of a signed int does the sign extension.
static uint32_t imm_model_i(uint32_t instr) {
  return (uint32_t)((int32_t)instr >> 20);
}

static void check(uint32_t instr, uint32_t expected, const char* what) {
  dut->instr = instr;
  dut->eval();
  wave->dump(ctx.time());
  ctx.timeInc(1);
  total++;
  if (dut->imm == expected) {
    passed++;
  } else {
    printf("FAIL %s: instr=0x%08x expected=0x%08x actual=0x%08x\n",
           what, instr, expected, dut->imm);
  }
}

int main(int argc, char** argv) {
  ctx.commandArgs(argc, argv);
  ctx.traceEverOn(true);
  dut = new Vimm_gen{&ctx};
  wave = new VerilatedVcdC;
  dut->trace(wave, 99);
  wave->open("build/imm_gen.vcd");

  // 1. Real instructions (encodings from the RISC-V assembler).
  check(0x00500093, 0x00000005, "addi x1, x0, 5");
  check(0xFFB00093, 0xFFFFFFFB, "addi x1, x0, -5");
  check(0x00000013, 0x00000000, "nop (addi x0, x0, 0)");
  check(0x00812283, 0x00000008, "lw x5, 8(x2)");
  check(0xFFC50067, 0xFFFFFFFC, "jalr x0, -4(x10)");

  // 2. Range limits: 0x7FF is the largest positive, 0x800 the most negative.
  check(0x7FF00093, 0x000007FF, "addi x1, x0, 2047");
  check(0x80000093, 0xFFFFF800, "addi x1, x0, -2048");

  // Formats not built yet produce 0.
  check(0x00112423, 0x00000000, "sw (S-type, not built yet)");

  // 3. Random I-type instructions: random upper 25 bits, I-type opcode.
  unsigned seed = 1;
  if (const char* s = getenv("SEED")) seed = strtoul(s, nullptr, 0);
  std::mt19937 rng(seed);
  for (int i = 0; i < 1000; i++) {
    uint32_t instr = (rng() & ~0x7Fu) | I_TYPE_OPCODES[rng() % 3];
    check(instr, imm_model_i(instr), "random I-type");
  }

  wave->close();
  dut->final();
  printf("IMM_GEN: %d/%d passed (seed %u)\n", passed, total, seed);
  return passed == total ? 0 : 1;
}
