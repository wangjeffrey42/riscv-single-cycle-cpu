<!-- Paste these sections into your README.md, under your existing one-sentence description. -->

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

- [ ] *Fill in what works now, e.g. "ALU supports all 10 operations and passes randomized tests in Verilator"*
- [ ] *Add a waveform screenshot: `docs/images/alu_waveform.png`*

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
