# Makefile for the RV32I single-cycle CPU
#
#   make test               build and run every testbench, exit nonzero on any failure
#   make run                interactive ALU calculator
#   make wave MODULE=alu    open a module's waveform in the viewer
#   make lint               check all RTL with verilator --lint-only -Wall
#   make clean              delete build/
#
# Conventions (so new modules need no Makefile changes):
#   rtl/<name>.sv        the hardware module
#   tb/<name>_tb.cpp     its C++ testbench
#   rtl/*_pkg.sv         shared packages (compiled before every module)
#   build/<name>/sim     the compiled testbench
#   build/<name>.vcd     the waveform the testbench writes

BUILD     := build
VERILATOR := verilator

# Every tb/<name>_tb.cpp is the testbench for rtl/<name>.sv.
MODULES := $(patsubst tb/%_tb.cpp,%,$(wildcard tb/*_tb.cpp))

# Every RTL module except packages (used by lint).
RTL_MODULES := $(filter-out %_pkg,$(patsubst rtl/%.sv,%,$(wildcard rtl/*.sv)))

# Packages must come before the modules that import them.
PKGS := $(wildcard rtl/*_pkg.sv)

# Shared C++ headers used by the testbenches.
TB_HEADERS := $(wildcard tb/*.h)

# --cc --exe --build : turn the SystemVerilog into C++, link it with our testbench, compile it
# --trace            : allow the testbench to write VCD waveforms
# -Wall              : turn on every lint warning (warnings stop the build)
VFLAGS := --cc --exe --build -j 0 -Wall --trace -CFLAGS -I$(CURDIR)/tb

# Waveform viewer: prefer Surfer, fall back to GTKWave.
VIEWER := $(shell command -v surfer || command -v gtkwave)

.PHONY: test run wave lint clean

# Build one module's testbench. Verilator's output is saved to build.log and
# only printed if the build fails, so `make test` stays readable.
# The .cpp path is absolute because older Verilators (e.g. Ubuntu's) compile
# it from inside build/<name>/, where a relative path wouldn't resolve.
$(BUILD)/%/sim: $(PKGS) rtl/%.sv tb/%_tb.cpp $(TB_HEADERS)
	@mkdir -p $(@D)
	@echo "  BUILD  $*"
	@$(VERILATOR) $(VFLAGS) --top-module $* -Mdir $(@D) -o sim \
		$(PKGS) rtl/$*.sv $(CURDIR)/tb/$*_tb.cpp > $(@D)/build.log 2>&1 \
		|| (cat $(@D)/build.log; exit 1)

# Running a testbench writes its waveform.
$(BUILD)/%.vcd: $(BUILD)/%/sim
	@./$< > /dev/null

test: $(MODULES:%=$(BUILD)/%/sim)
	@echo "  RUN    tests"
	@fail=0; \
	for m in $(MODULES); do \
		./$(BUILD)/$$m/sim || fail=1; \
	done; \
	if [ $$fail -eq 0 ]; then echo "All modules passed."; \
	else echo "Some modules FAILED."; exit 1; fi

# Interactive ALU: same RTL as the ALU test, different C++ main.
$(BUILD)/alu_repl/sim: $(PKGS) rtl/alu.sv tb/alu_repl.cpp $(TB_HEADERS)
	@mkdir -p $(@D)
	@echo "  BUILD  alu_repl"
	@$(VERILATOR) $(VFLAGS) --top-module alu -Mdir $(@D) -o sim \
		$(PKGS) rtl/alu.sv $(CURDIR)/tb/alu_repl.cpp > $(@D)/build.log 2>&1 \
		|| (cat $(@D)/build.log; exit 1)

run: $(BUILD)/alu_repl/sim
	@./$(BUILD)/alu_repl/sim

wave:
	@if [ -z "$(MODULE)" ]; then \
		echo "Usage: make wave MODULE=<name>   (one of: $(MODULES))"; exit 1; fi
	@if [ -z "$(VIEWER)" ]; then \
		echo "No waveform viewer found. Install surfer or gtkwave."; exit 1; fi
	@$(MAKE) --no-print-directory $(BUILD)/$(MODULE).vcd
	$(VIEWER) $(BUILD)/$(MODULE).vcd

lint:
	@for m in $(RTL_MODULES); do \
		echo "  LINT   $$m"; \
		$(VERILATOR) --lint-only -Wall --top-module $$m $(PKGS) rtl/$$m.sv || exit 1; \
	done
	@echo "Lint clean."

clean:
	rm -rf $(BUILD)
