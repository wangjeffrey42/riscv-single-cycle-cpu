// pc_reg: the Program Counter register. It holds the address of the
// instruction the CPU is running right now.
//
// This is sequential logic: it has memory. It only changes on the rising
// edge of the clock (when clk goes 0 -> 1); between edges it holds its value
// no matter what pc_next does. That is what lets a single-cycle CPU run one
// instruction per clock tick: during the cycle the rest of the CPU computes
// pc_next (usually pc + 4, or a jump target), and at the next edge pc takes it.
//
// Ports:
//   clk      the clock
//   rst      reset, active high: while 1, the next edge sets pc to 0
//   pc_next  the address of the next instruction (computed elsewhere)
//   pc       the address of the current instruction

module pc_reg (
  input  logic        clk,
  input  logic        rst,
  input  logic [31:0] pc_next,
  output logic [31:0] pc
);

  // always_ff describes a flip-flop (a register). Use <= ("nonblocking
  // assignment") here: every register updates at the same instant on the edge.
  //
  // This is a synchronous reset: rst is only looked at on a clock edge.
  // Programs start at address 0, so that's where reset sends the PC.
  always_ff @(posedge clk) begin
    if (rst) pc <= 32'b0;
    else     pc <= pc_next;
  end

endmodule
