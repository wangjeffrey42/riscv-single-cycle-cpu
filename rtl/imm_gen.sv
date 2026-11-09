// imm_gen: the Immediate Generator. It pulls the constant ("immediate")
// out of an instruction and sign-extends it to 32 bits.
//
// In `addi x1, x0, -5`, the -5 is stored inside the 32-bit instruction itself,
// in only 12 bits. The ALU needs a full 32-bit value, so we copy the top bit
// (the sign bit) of those 12 bits into the upper 20 bits. This is sign
// extension: 12-bit -5 is 0xFFB, and as 32 bits it becomes 0xFFFFFFFB.
//
// RISC-V has 5 immediate formats (I, S, B, U, J); each stores the bits in
// different places in the instruction. Only I-type is built so far. The
// format is picked from the opcode (instr[6:0]) so the other formats can be
// added as new case items.
//
// Ports:
//   instr  the 32-bit instruction
//   imm    the sign-extended 32-bit immediate (0 for formats not built yet)

module imm_gen (
  // Bits instr[19:7] (registers and funct3) aren't used by I-type immediates.
  // They will be once S, B, U, and J types are added, so silence the warning.
  /* verilator lint_off UNUSEDSIGNAL */
  input  logic [31:0] instr,
  /* verilator lint_on UNUSEDSIGNAL */
  output logic [31:0] imm
);

  // Opcodes that use the I-type format.
  localparam logic [6:0] OP_IMM = 7'b0010011;  // addi, slti, xori, ...
  localparam logic [6:0] LOAD   = 7'b0000011;  // lw, lh, lb, ...
  localparam logic [6:0] JALR   = 7'b1100111;  // jalr

  // Instruction layout for I-type:
  //   31        20 19   15 14  12 11    7 6      0
  //   [ imm[11:0] ][  rs1  ][funct3][  rd  ][ opcode ]
  always_comb begin
    case (instr[6:0])
      // I-type: imm[11:0] = instr[31:20]. {20{instr[31]}} repeats the sign bit 20 times.
      OP_IMM, LOAD, JALR: imm = {{20{instr[31]}}, instr[31:20]};

      // To add later:
      //   S-type (stores):   imm = {{20{instr[31]}}, instr[31:25], instr[11:7]};
      //   B-type (branches): imm = {{19{instr[31]}}, instr[31], instr[7], instr[30:25], instr[11:8], 1'b0};
      //   U-type (lui):      imm = {instr[31:12], 12'b0};
      //   J-type (jal):      imm = {{11{instr[31]}}, instr[31], instr[19:12], instr[20], instr[30:21], 1'b0};

      default: imm = 32'b0;
    endcase
  end

endmodule
