# MVP Demo

**Video:** [MVP demo recording](PASTE-VIDEO-LINK-HERE)
**Presented:** October 22, 2026
**Release:** [v0.1.0](PASTE-RELEASE-LINK-HERE)

## What the demo shows

- `make test` building and running the testbenches for the ALU, program counter, register file, and immediate generator, all passing
- `make run`: typing ALU operations like `sub 5 7` and `sltu 0xFFFFFFFF 1` and seeing the hardware's answer
- `make wave MODULE=pc_reg`: the program counter stepping 0, 4, 8, ... on each rising clock edge
- The CI badge on GitHub, showing the same tests run automatically on every push

## How to run it yourself

Install Verilator 5 and a waveform viewer first (see the README's "Build and Run" section).

```bash
git clone https://github.com/wangjeffrey42/riscv-single-cycle-cpu.git
cd riscv-single-cycle-cpu
make test                 # build and run every testbench
make run                  # interactive ALU (type 'help', 'quit' to exit)
make wave MODULE=pc_reg   # open the program counter's waveform
make lint                 # check the hardware code for warnings
```

## Planned for the next iteration

- Control unit and ALU control
- Instruction and data memories, and the full single-cycle datapath (`cpu_top`)
- First programs running end to end (arithmetic instructions)
- Goal by end of quarter: all 37 RV32I instructions passing official tests
