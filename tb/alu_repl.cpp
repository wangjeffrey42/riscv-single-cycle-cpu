// alu_repl.cpp: an interactive calculator that runs on the real ALU hardware.
//
// Type an operation and two numbers; the values go into the simulated ALU
// (rtl/alu.sv) and its output is printed. Started with `make run`.
//
//   alu> sub 5 7
//   alu> sltu 0xFFFFFFFF 1

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "Valu.h"
#include "alu_model.h"
#include "verilated.h"

// Parse a decimal (possibly negative) or 0x-hex number that fits in 32 bits.
// Returns false if the text isn't a valid number.
static bool parse_u32(const std::string& text, uint32_t& out) {
  const char* s = text.c_str();
  bool negative = (*s == '-');
  const char* digits = negative ? s + 1 : s;
  int base = (strncmp(digits, "0x", 2) == 0 || strncmp(digits, "0X", 2) == 0) ? 16 : 10;
  if (*digits == '\0' || *digits == '-' || *digits == '+') return false;

  char* end;
  long long value = strtoll(digits, &end, base);
  if (*end != '\0') return false;
  if (negative) value = -value;
  if (value < INT32_MIN || value > (long long)UINT32_MAX) return false;
  out = (uint32_t)value;  // negative numbers become their two's complement bits
  return true;
}

static void print_help() {
  printf("Usage: <op> <a> <b>     numbers can be decimal (-5) or hex (0xFF)\n");
  printf("Operations:\n");
  printf("  add sub         a + b, a - b\n");
  printf("  and or xor      bitwise\n");
  printf("  sll srl sra     shift a by b[4:0] (left, right with 0s, right with sign bit)\n");
  printf("  slt sltu        1 if a < b (signed / unsigned), else 0\n");
  printf("Other commands: help, quit\n");
}

int main(int argc, char** argv) {
  VerilatedContext ctx;
  ctx.commandArgs(argc, argv);
  Valu dut{&ctx};

  printf("RV32I ALU (simulated hardware). Type 'help' for operations, 'quit' to exit.\n");
  std::string line;
  while (true) {
    printf("alu> ");
    fflush(stdout);
    if (!std::getline(std::cin, line)) break;  // Ctrl-D

    std::istringstream in(line);
    std::string op_name, a_text, b_text, extra;
    if (!(in >> op_name)) continue;  // empty line
    if (op_name == "quit" || op_name == "exit") break;
    if (op_name == "help") { print_help(); continue; }

    int op = -1;
    for (int i = 0; i < ALU_NUM_OPS; i++) {
      if (op_name == alu_op_name(i)) op = i;
    }
    if (op < 0) { printf("Unknown operation '%s'. Type 'help'.\n", op_name.c_str()); continue; }

    uint32_t a, b;
    if (!(in >> a_text >> b_text) || (in >> extra)) {
      printf("Need exactly two numbers, e.g. '%s 5 7'.\n", op_name.c_str());
      continue;
    }
    if (!parse_u32(a_text, a) || !parse_u32(b_text, b)) {
      printf("Numbers must be decimal or 0x hex and fit in 32 bits.\n");
      continue;
    }

    // Drive the hardware and let it compute.
    dut.a = a;
    dut.b = b;
    dut.alu_ctrl = op;
    dut.eval();

    uint32_t r = dut.result;
    printf("  hex 0x%08x   signed %d   unsigned %u   zero=%d\n", r, (int32_t)r, r, dut.zero);
  }

  dut.final();
  return 0;
}
