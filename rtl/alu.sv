// alu: the Arithmetic Logic Unit. It does the math for every instruction.
//
// It is purely combinational: there is no clock. Whenever an input changes,
// the output changes right after, like a C++ function that is re-evaluated
// constantly. The CPU uses it for arithmetic (add, sub), logic (and, or, xor),
// shifts, comparisons (slt, sltu), and later for computing memory addresses.
//
// Ports:
//   a        first operand (from register rs1)
//   b        second operand (from register rs2, or an immediate)
//   alu_ctrl which operation to do (one of the names in alu_pkg)
//   result   the answer
//   zero     1 when result is 0; branches will use this to compare values

module alu
  import alu_pkg::*;
(
  input  logic [31:0] a,
  input  logic [31:0] b,
  input  alu_op_e     alu_ctrl,
  output logic [31:0] result,
  output logic        zero
);

  // A 32-bit value can only be shifted by 0..31, so RISC-V uses only the
  // low 5 bits of b as the shift amount and ignores the rest.
  logic [4:0] shamt;
  assign shamt = b[4:0];

  // always_comb describes combinational logic. Every path must assign
  // `result`; otherwise the tool would have to "remember" the old value and
  // would build a latch (accidental memory). The default case prevents that.
  always_comb begin
    case (alu_ctrl)
      ALU_ADD:  result = a + b;              // overflow wraps, like uint32_t
      ALU_SUB:  result = a - b;
      ALU_AND:  result = a & b;
      ALU_OR:   result = a | b;
      ALU_XOR:  result = a ^ b;
      ALU_SLL:  result = a << shamt;
      ALU_SRL:  result = a >> shamt;         // >> always fills with 0
      // >>> fills with the sign bit, but only if the operand is signed.
      // $signed() tells the tool to treat a as a two's complement number.
      ALU_SRA:  result = $unsigned($signed(a) >>> shamt);
      // Comparisons give 1 or 0, padded to 32 bits with zeros.
      // Same bits, different meaning: 0xFFFFFFFF is -1 signed but 4294967295 unsigned.
      ALU_SLT:  result = {31'b0, $signed(a) < $signed(b)};
      ALU_SLTU: result = {31'b0, a < b};
      default:  result = 32'b0;              // unused codes 10..15
    endcase
  end

  assign zero = (result == 32'b0);

endmodule
