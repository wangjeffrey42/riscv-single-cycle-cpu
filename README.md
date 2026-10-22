# riscv-single-cycle-cpu

**Software product:** a single-cycle 32-bit RISC-V (RV32I) processor that runs real RISC-V programs in simulation.
**Tech stack:** SystemVerilog, Verilator, C++ testbenches, Make, Surfer/GTKWave, RISC-V GNU toolchain.

## About the Project

This project is a working RISC-V processor built from scratch in SystemVerilog. It runs programs written for the RV32I instruction set, the base version of RISC-V, an open instruction set used in everything from microcontrollers to research chips. Each part of the CPU (the ALU, register file, immediate generator, control unit, and memories) is its own module with its own tests, so anyone can see exactly how a computer turns an instruction into action. The goal is a core that passes all 37 RV32I instructions against the official RISC-V specification, runs small compiled C programs, and is simple enough to read, learn from, and extend.

## User Roles

**Learners** (students studying computer architecture)
- Run a RISC-V program on the CPU and watch every signal change in a waveform viewer
- Read a short, commented module for each CPU part to see how it works
- Compare what each instruction does in the spec with what the hardware does

**Program writers** (people writing RISC-V software)
- Write a RISC-V assembly or C program, compile it to a hex file, and run it on the CPU
- Check the final register and memory values to confirm the program worked

**Hardware developers and researchers**
- Run the full test suite with one command to confirm the CPU is correct
- Extend the core, for example by adding instructions, a pipeline, or an FPGA build
- Use it as a simple, open base for experiments, such as measuring energy or controlling a hardware accelerator

The project is a public code repository with no accounts or user-contributed content, so no permission system is needed.

## Current State (MVP)

The project is in the setup and design phase. No CPU modules are written yet.

- [x] Repository set up with README, MIT license, and `.gitignore`
- [x] CPU architecture planned: module hierarchy and datapath (below)
- [ ] Toolchain installed and verified (Verilator, RISC-V GNU toolchain, Surfer/GTKWave)
- [ ] First module (ALU) written and tested

## Architecture

### Module hierarchy

Each module will be its own SystemVerilog file in `rtl/`, with its own testbench in `tb/`.

```mermaid
flowchart TD
    top["cpu_top<br/><i>wires everything together</i>"]
    top --> pc["pc_reg<br/><i>program counter</i>"]
    top --> imem["imem<br/><i>instruction memory</i>"]
    top --> ctrl["control<br/><i>main control unit</i>"]
    top --> rf["regfile<br/><i>32 x 32-bit registers</i>"]
    top --> imm["imm_gen<br/><i>immediate generator</i>"]
    top --> aluc["alu_control<br/><i>picks the ALU operation</i>"]
    top --> alu["alu<br/><i>math and logic</i>"]
    top --> br["branch_unit<br/><i>branch comparisons</i>"]
    top --> dmem["dmem<br/><i>data memory</i>"]
```

### Datapath (one instruction per clock cycle)

```mermaid
flowchart LR
    PC[PC] --> IMEM[Instruction Memory]
    IMEM --> CTRL[Control]
    IMEM --> RF[Register File]
    IMEM --> IMM[Imm Gen]
    RF -- rs1 --> ALU
    RF -- rs2 --> MUX{ALUSrc}
    IMM --> MUX
    MUX --> ALU[ALU]
    ALU -- address --> DMEM[Data Memory]
    RF -- rs2 --> DMEM
    ALU --> WB{Write-back}
    DMEM --> WB
    WB -- rd --> RF
    RF --> BR[Branch Unit]
    BR --> NPC{Next PC}
    NPC --> PC
```

| Module | Job |
| --- | --- |
| `pc_reg` | Holds the address of the current instruction; updates every clock edge |
| `imem` | Read-only memory loaded from a `.hex` program file |
| `control` | Reads the opcode and sets the control signals (register write, memory read/write, branch, ...) |
| `regfile` | 32 registers with two read ports and one write port; `x0` is always zero |
| `imm_gen` | Extracts and sign-extends the immediate for the I, S, B, U, and J formats |
| `alu_control` | Turns `funct3`/`funct7` into the exact ALU operation |
| `alu` | Add, subtract, AND, OR, XOR, shifts, and comparisons |
| `branch_unit` | Decides whether `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu` are taken |
| `dmem` | Data memory for loads and stores (byte, halfword, word) |
| `cpu_top` | Connects all modules, plus the PC+4 adder, branch target adder, and muxes |

### Planned repo layout

```
riscv-single-cycle-cpu/
├── rtl/        SystemVerilog modules (alu.sv, regfile.sv, ...)
├── tb/         Verilator C++ testbenches, one per module
├── programs/   RISC-V assembly and C test programs
├── docs/       Diagrams, demo notes, waveform screenshots
├── Makefile    make test, make run, make wave
├── README.md
└── LICENSE.md
```

## Roadmap

1. **Setup and design:** repo, toolchain, architecture plan *(current)*
2. **Core modules:** ALU, register file, immediate generator, program counter, each with tests
3. **Control:** main control unit and ALU control
4. **Full datapath:** memories and `cpu_top`; first arithmetic programs running end to end
5. **Full ISA:** loads, stores, branches, and jumps; all 37 RV32I instructions
6. **Verification:** official `riscv-tests` suite and comparison against Spike
7. **Real programs:** small C programs compiled with GCC

## Tech Stack

| Technology | What it's for | Why I chose it |
| --- | --- | --- |
| **SystemVerilog** | Describing the CPU hardware | Industry-standard hardware language; cleaner than plain Verilog (`logic`, `always_ff`, `always_comb`) |
| **Verilator** | Simulating the design | Free, fast, and widely used in industry and open-source RISC-V projects |
| **C++ testbenches** | Feeding inputs and checking outputs | Verilator compiles the design to C++, and I already know C++ from CMPSC 16 and 24 |
| **Make** | Building and running tests | One command (`make test`) runs everything |
| **Surfer / GTKWave** | Viewing waveforms | Free waveform viewers for debugging signal timing |
| **RISC-V GNU toolchain, Venus** | Turning assembly and C into machine code | Produces the programs the CPU runs |
| **Spike** | Reference simulator | Official RISC-V simulator to compare results against |

I may also explore building the same CPU in **PyRTL**, a Python hardware library from UCSB's ArchLab, to compare the two approaches.

## License

[MIT](LICENSE.md)
