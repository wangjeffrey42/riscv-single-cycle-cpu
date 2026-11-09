// pc_reg_tb.cpp: tests rtl/pc_reg.sv.
//
// Acts like the rest of the CPU: every cycle it sets pc_next = pc + 4, so the
// PC should count 0, 4, 8, 12, ... Then it asserts reset in the middle of the
// count and checks the PC goes back to 0 and starts counting again.
//
// Writes build/pc_reg.vcd: open it to watch pc step up on each rising clk edge.

#include <cstdio>

#include "Vpc_reg.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

static VerilatedContext ctx;
static Vpc_reg* dut;
static VerilatedVcdC* wave;
static int passed = 0, total = 0;

// One full clock cycle: clk low for 5 time units, then high for 5.
// The register updates at the 0 -> 1 transition.
static void tick() {
  dut->clk = 0;
  dut->eval();
  wave->dump(ctx.time());
  ctx.timeInc(5);
  dut->clk = 1;
  dut->eval();
  wave->dump(ctx.time());
  ctx.timeInc(5);
}

static void expect_pc(uint32_t expected, const char* what) {
  total++;
  if (dut->pc == expected) {
    passed++;
  } else {
    printf("FAIL %s: expected pc=0x%08x actual pc=0x%08x\n", what, expected, dut->pc);
  }
}

int main(int argc, char** argv) {
  ctx.commandArgs(argc, argv);
  ctx.traceEverOn(true);
  dut = new Vpc_reg{&ctx};
  wave = new VerilatedVcdC;
  dut->trace(wave, 99);
  wave->open("build/pc_reg.vcd");

  // Reset: pc starts as garbage, one clock edge with rst = 1 makes it 0.
  dut->rst = 1;
  dut->pc_next = 0xDEADBEEF;  // must be ignored during reset
  tick();
  expect_pc(0, "after reset");
  dut->rst = 0;

  // Count for 20 cycles: 0 -> 4 -> 8 -> ... -> 80.
  for (uint32_t i = 1; i <= 20; i++) {
    dut->pc_next = dut->pc + 4;
    tick();
    expect_pc(4 * i, "counting");
  }

  // Reset in the middle of counting.
  dut->rst = 1;
  dut->pc_next = dut->pc + 4;
  tick();
  expect_pc(0, "mid-count reset");
  dut->rst = 0;

  // Counting resumes from 0.
  for (uint32_t i = 1; i <= 5; i++) {
    dut->pc_next = dut->pc + 4;
    tick();
    expect_pc(4 * i, "counting after reset");
  }

  // Between edges the register holds its value even if pc_next changes.
  dut->pc_next = 0x12345678;
  dut->clk = 0;
  dut->eval();
  expect_pc(20, "hold between edges");

  wave->close();
  dut->final();
  printf("PC_REG:  %d/%d passed\n", passed, total);
  return passed == total ? 0 : 1;
}
