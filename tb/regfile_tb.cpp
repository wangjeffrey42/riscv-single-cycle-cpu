// regfile_tb.cpp: tests rtl/regfile.sv against a plain C++ array.
//
// 1. Write a different value to every register, then read them all back.
// 2. Write 99 to x0 and check it still reads 0.
// 3. With we = 0, a clock edge must not change anything.
// 4. Same-cycle write and read: the read sees the old value until the edge.
// 5. 5,000 random cycles of reads and writes, compared against the model.
//
// Writes build/regfile.vcd.

#include <cstdio>
#include <cstdlib>
#include <random>

#include "Vregfile.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

// Created in main(), not as a global object: globals in different files are
// constructed in no fixed order, and Verilator needs its own globals first.
static VerilatedContext* ctx;
static Vregfile* dut;
static VerilatedVcdC* wave;
static int passed = 0, total = 0;

// The reference model: 32 registers, x0 is always 0.
static uint32_t model[32] = {0};

static void tick() {
  dut->clk = 0;
  dut->eval();
  wave->dump(ctx->time());
  ctx->timeInc(5);
  dut->clk = 1;
  dut->eval();
  wave->dump(ctx->time());
  ctx->timeInc(5);
}

// Write one register through the write port (takes one clock cycle).
static void write_reg(uint32_t rd, uint32_t value) {
  dut->we = 1;
  dut->rd = rd;
  dut->wd = value;
  tick();
  dut->we = 0;
  if (rd != 0) model[rd] = value;
}

// Read two registers through the read ports (no clock needed) and compare.
static void check_read(uint32_t rs1, uint32_t rs2, const char* what) {
  dut->rs1 = rs1;
  dut->rs2 = rs2;
  dut->eval();
  total++;
  if (dut->rd1 == model[rs1] && dut->rd2 == model[rs2]) {
    passed++;
  } else {
    printf("FAIL %s: rs1=x%u expected=0x%08x actual=0x%08x | rs2=x%u expected=0x%08x actual=0x%08x\n",
           what, rs1, model[rs1], dut->rd1, rs2, model[rs2], dut->rd2);
  }
}

int main(int argc, char** argv) {
  ctx = new VerilatedContext;
  ctx->commandArgs(argc, argv);
  ctx->traceEverOn(true);
  dut = new Vregfile{ctx};
  wave = new VerilatedVcdC;
  dut->trace(wave, 99);
  wave->open("build/regfile.vcd");
  dut->we = 0;

  // 1. Write every register with a value that encodes its number, read back.
  for (uint32_t r = 1; r < 32; r++) write_reg(r, 0x1000 + r);
  for (uint32_t r = 0; r < 32; r++) check_read(r, 31 - r, "write then read");

  // 2. x0 stays 0.
  write_reg(0, 99);
  check_read(0, 0, "x0 after writing 99");

  // 3. we = 0: put a value on the write port but don't enable it.
  dut->we = 0;
  dut->rd = 5;
  dut->wd = 0xBAD0BAD0;
  tick();
  check_read(5, 5, "we = 0 does not write");

  // 4. Same cycle: before the edge we read the old value of x7, after it the new one.
  dut->we = 1;
  dut->rd = 7;
  dut->wd = 0xCAFEF00D;
  dut->clk = 0;
  check_read(7, 7, "same-cycle read returns old value");
  tick();
  dut->we = 0;
  model[7] = 0xCAFEF00D;
  check_read(7, 7, "read after the edge returns new value");

  // 5. Random reads and writes.
  unsigned seed = 1;
  if (const char* s = getenv("SEED")) seed = strtoul(s, nullptr, 0);
  std::mt19937 rng(seed);
  for (int i = 0; i < 5000; i++) {
    // One rng() call per line so the same seed gives the same test everywhere.
    if (rng() % 2) {
      uint32_t rd = rng() % 32;
      uint32_t value = rng();
      write_reg(rd, value);
    }
    uint32_t rs1 = rng() % 32;
    uint32_t rs2 = rng() % 32;
    check_read(rs1, rs2, "random");
  }

  wave->close();
  dut->final();
  printf("REGFILE: %d/%d passed (seed %u)\n", passed, total, seed);
  return passed == total ? 0 : 1;
}
