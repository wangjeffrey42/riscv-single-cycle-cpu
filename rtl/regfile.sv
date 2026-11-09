// regfile: the 32 general-purpose registers x0..x31, 32 bits each.
//
// This is the CPU's fastest storage. An instruction like `add x3, x1, x2`
// reads two registers (x1, x2) and writes one (x3), so the register file has
// two read ports and one write port.
//
// x0 is special in RISC-V: it always reads as 0, and writes to it are thrown
// away. That gives programs a free constant 0 (e.g. `addi x1, x0, 5` means
// x1 = 0 + 5).
//
// Ports:
//   clk   the clock; writes happen on its rising edge
//   rs1   address (0..31) of the first register to read
//   rs2   address (0..31) of the second register to read
//   rd1   value of register rs1
//   rd2   value of register rs2
//   we    write enable: 1 = write wd into register rd on the next rising edge
//   rd    address (0..31) of the register to write
//   wd    the value to write
//
// Reading and writing the same register in the same cycle:
//   Reads are combinational, so rd1/rd2 show what is stored *right now*. The
//   write only happens at the rising edge, at the end of the cycle. So during
//   that cycle a read of register rd returns the OLD value; the new value
//   shows up after the edge. In a single-cycle CPU this is exactly what we
//   want: each instruction reads its inputs and writes its result at the end.

module regfile (
  input  logic        clk,
  input  logic [4:0]  rs1,
  input  logic [4:0]  rs2,
  output logic [31:0] rd1,
  output logic [31:0] rd2,
  input  logic        we,
  input  logic [4:0]  rd,
  input  logic [31:0] wd
);

  // An array of 32 registers, each 32 bits wide. regs[0] is never written.
  logic [31:0] regs [32];

  // Write port (sequential): only on a clock edge, only if enabled, never x0.
  always_ff @(posedge clk) begin
    if (we && rd != 5'd0) regs[rd] <= wd;
  end

  // Read ports (combinational): x0 is hard-wired to 0.
  assign rd1 = (rs1 == 5'd0) ? 32'b0 : regs[rs1];
  assign rd2 = (rs2 == 5'd0) ? 32'b0 : regs[rs2];

endmodule
