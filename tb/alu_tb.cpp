// alu_tb.cpp: tests rtl/alu.sv against the C++ model in alu_model.h.
//
// 1. Directed tests: hand-picked edge cases where bugs usually hide.
// 2. Random tests: 10,000 random (op, a, b) triples. The seed is fixed so a
//    failure is repeatable; set SEED=<number> to try a different set.
//
// Each test is one time step in build/alu.vcd, so you can scroll through
// them in the waveform viewer.

#include <cstdio>
#include <cstdlib>
#include <random>

#include "Valu.h"
#include "alu_model.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

// Created in main(), not as a global object: globals in different files are
// constructed in no fixed order, and Verilator needs its own globals first.
static VerilatedContext* ctx;
static Valu* dut;
static VerilatedVcdC* wave;
static int passed = 0, total = 0;

// Drive the inputs, let the combinational logic settle, and compare.
static void check(int op, uint32_t a, uint32_t b) {
  dut->a = a;
  dut->b = b;
  dut->alu_ctrl = op;
  dut->eval();
  wave->dump(ctx->time());
  ctx->timeInc(1);

  uint32_t expected = alu_model(op, a, b);
  bool expected_zero = (expected == 0);
  total++;
  if (dut->result == expected && dut->zero == expected_zero) {
    passed++;
  } else {
    printf("FAIL %-4s a=0x%08x b=0x%08x  expected=0x%08x zero=%d  actual=0x%08x zero=%d\n",
           alu_op_name(op), a, b, expected, expected_zero, dut->result, dut->zero);
  }
}

int main(int argc, char** argv) {
  ctx = new VerilatedContext;
  ctx->commandArgs(argc, argv);
  ctx->traceEverOn(true);
  dut = new Valu{ctx};
  wave = new VerilatedVcdC;
  dut->trace(wave, 99);
  wave->open("build/alu.vcd");

  // --- Directed tests ---
  check(ALU_ADD, 0x7FFFFFFF, 1);           // overflow wraps to 0x80000000
  check(ALU_ADD, 0xFFFFFFFF, 1);           // wraps to 0, so zero = 1
  check(ALU_ADD, 0, 0);
  check(ALU_SUB, 0, 1);                    // 0 - 1 = 0xFFFFFFFF
  check(ALU_SUB, 5, 5);                    // equal values give zero = 1 (branches use this)
  check(ALU_SUB, 0x80000000, 1);           // most negative - 1 wraps to 0x7FFFFFFF
  check(ALU_AND, 0xF0F0F0F0, 0xFF00FF00);
  check(ALU_OR,  0xF0F0F0F0, 0x0F0F0F0F);
  check(ALU_XOR, 0xFFFFFFFF, 0xFFFFFFFF);  // x ^ x = 0
  for (uint32_t s : {0u, 1u, 31u}) {       // shift by 0, 1, and 31
    check(ALU_SLL, 0x80000001, s);
    check(ALU_SRL, 0x80000001, s);
    check(ALU_SRA, 0x80000001, s);
  }
  check(ALU_SLL, 1, 32);                   // only b[4:0] is used: 32 acts like 0
  check(ALU_SRL, 0x80000000, 0xFFFFFFE4);  // b[4:0] = 4
  check(ALU_SRA, 0xF0000000, 4);           // negative: SRA gives 0xFF000000
  check(ALU_SRL, 0xF0000000, 4);           //           SRL gives 0x0F000000
  check(ALU_SRA, 0x70000000, 4);           // positive: both give 0x07000000
  check(ALU_SLT,  0xFFFFFFFF, 1);          // -1 < 1 signed: 1
  check(ALU_SLTU, 0xFFFFFFFF, 1);          // 4294967295 < 1 unsigned: 0
  check(ALU_SLT,  1, 0xFFFFFFFF);
  check(ALU_SLTU, 1, 0xFFFFFFFF);
  check(ALU_SLT,  0x80000000, 0x7FFFFFFF); // most negative < most positive
  check(ALU_SLT,  7, 7);                   // equal is not less than
  check(ALU_SLTU, 7, 7);

  // --- Random tests ---
  unsigned seed = 1;
  if (const char* s = getenv("SEED")) seed = strtoul(s, nullptr, 0);
  std::mt19937 rng(seed);
  for (int i = 0; i < 10000; i++) {
    // One rng() call per line: C++ doesn't fix the order function arguments
    // are evaluated in, so check(rng(), rng(), rng()) would give different
    // tests for the same seed on different compilers.
    int op = rng() % ALU_NUM_OPS;
    uint32_t a = rng();
    uint32_t b = rng();
    check(op, a, b);
  }

  wave->close();
  dut->final();
  printf("ALU:     %d/%d passed (seed %u)\n", passed, total, seed);
  return passed == total ? 0 : 1;
}
