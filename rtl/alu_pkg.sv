// alu_pkg: names for the ALU operations.
//
// A package is SystemVerilog's version of a shared header file. Putting the
// operation codes here means the ALU and (later) the ALU control unit both
// say ALU_ADD instead of the magic number 4'd0, and can never disagree.
//
// The numbers themselves are our own choice; they are not part of the RISC-V
// spec. The C++ testbenches mirror them in tb/alu_model.h.

package alu_pkg;

  // 4 bits gives room for 16 operations; RV32I needs 10.
  typedef enum logic [3:0] {
    ALU_ADD  = 4'd0,  // a + b
    ALU_SUB  = 4'd1,  // a - b
    ALU_AND  = 4'd2,  // a & b
    ALU_OR   = 4'd3,  // a | b
    ALU_XOR  = 4'd4,  // a ^ b
    ALU_SLL  = 4'd5,  // shift left logical:      a << b[4:0]
    ALU_SRL  = 4'd6,  // shift right logical:     a >> b[4:0], fill with 0
    ALU_SRA  = 4'd7,  // shift right arithmetic:  a >> b[4:0], fill with a's sign bit
    ALU_SLT  = 4'd8,  // set less than (signed):   1 if a < b else 0
    ALU_SLTU = 4'd9   // set less than (unsigned): 1 if a < b else 0
  } alu_op_e;

endpackage
