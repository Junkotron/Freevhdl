# ========================================================================
# ⚔️ THE DEFINITIVE COMPLIANT PRINTF PRODUCTION MAKEFILE (ALL -e REMOVED)
# ========================================================================

YOSYS = yosys
CXX   = g++
GHDL  = ghdl

# ------------------------------------------------------------------------
# 🎛️ 1. PARAMETER-DEFAULTS & VERBOSE CONTROL
# ------------------------------------------------------------------------
PLATFORM ?= ice40
MODE      ?= help
V         ?= 0

ifeq ($(V),1)
    Q =
else
    Q = @
endif

# Helautomatisk färgdetektering via tput
HAS_COLORS := $(shell tput colors 2>/dev/null || echo 0)
ifeq ($(shell test $(HAS_COLORS) -ge 8 && echo 1 || echo 0),1)
    CLR_YELLOW = \033[1;33m
    CLR_RED    = \033[1;31m
    CLR_RESET  = \033[0m
else
    CLR_YELLOW =
    CLR_RED    =
    CLR_RESET  =
endif

OBJ_DIR        = obj_dir
GENERATED_SRC  = $(OBJ_DIR)/my6502_core_sim.cpp
GENERATED_OBJ  = $(OBJ_DIR)/my6502_core_sim.o
MAIN_OBJ       = $(OBJ_DIR)/main.o
TARGET_CXX     = simple_6502_sim

# LOGFILES
SYNTHLOG=$(OBJ_DIR)/synth_$(PLATFORM).log
PNRLOG=$(OBJ_DIR)/pnr_$(PLATFORM).log


# helper functions
# Dold felhanteringsfunktion för snyggare terminalutskrifter

define PIPE_HANDLE_ERROR_GOWIN
|| (printf "\n$(CLR_RED)❌ [HAREWERK-CRASH] $(PLATFORM) PnR misslyckades! Felorsak:$(CLR_RESET)\n"; \
    grep -E -i "error|unable" $(PNRLOG) 2>/dev/null || echo "Ingen felrad hittades."; \
    exit 1)
endef

# Dold felhanterare för iCE40 PnR
define PIPE_HANDLE_ERROR_ICE40
|| (printf "\n$(CLR_RED)❌ [HAREWERK-CRASH] iCE40 PnR misslyckades! Felrader:$(CLR_RESET)\n"; \
    grep -i "error" $(PNRLOG) 2>/dev/null | sed "s/ERROR/$(shell printf '$(CLR_RED)ERROR$(CLR_RESET)')/g" || grep -i "error" $(PNRLOG); \
    exit 1)
endef

define PIPE_HANDLE_ERROR_SYNTH
|| (printf "\n$(CLR_RED)❌ [HAREWERK-CRASH] Syntesen misslyckades! Felrader i loggen:$(CLR_RESET)\n"; \
    grep -i "error" $(SYNTHLOG) 2>/dev/null | sed "s/ERROR/$(shell printf '$(CLR_RED)ERROR$(CLR_RESET)')/g" | sed "s/error/$(shell printf '$(CLR_RED)error$(CLR_RESET)')/g" || grep -i "error" $(SYNTHLOG); \
    exit 1)
endef


# Flaggor från ditt originalmanus
GHDL_FLAGS = -fexplicit -fsynopsys --std=08

# --- HÅRDVARUFACIT FÖR DE OLIKA KORTEN ---
ICE40_DEVICE  = hx8k
ICE40_PACKAGE = ct256
ICE40_PCF     = my6502.pcf

GOWIN9K_DEVICE = GW1NR-LV9QN88PC6/I5
GOWIN9K_BOARD  = littlebee9k
GOWIN9K_FAMILY = GW1N-9

GOWIN20K_DEVICE = GW2AR-LV18QN88C8/I7
GOWIN20K_BOARD  = tang20k
GOWIN20K_FAMILY = GW2A-18C

# ------------------------------------------------------------------------
# 📜 2. DYNAMISK STRUKTUR-ROUTING UTIFRÅN VALT KORT
# ------------------------------------------------------------------------
CXXRTL_INC := $(shell $(YOSYS)-config --datdir)/include/backends/cxxrtl/runtime
CXXFLAGS   = -O3 -std=c++17 -I$(CXXRTL_INC) -I.

VHDL_CORE_SRCS = Oric_MiSTer/rtl/T65/T65_Pack.vhd \
                 Oric_MiSTer/rtl/T65/T65_MCode.vhd \
                 Oric_MiSTer/rtl/T65/T65_ALU.vhd \
                 Oric_MiSTer/rtl/T65/T65.vhd \
                 reset_logic.vhd \
                 divn.vhd \
                 my6502.vhd


# --- Standardvärden (Fallbacks om inget matchar) ---
VHDL_HARDWARE_SRCS = $(VHDL_CORE_SRCS)
TOPMODULE          = top_t65_system
SYNTH_TYPE         = ice40

# --- Specifik konfiguration per plattform ---
ifeq ($(PLATFORM),gowin9k)
    DEVICE             = $(GOWIN9K_DEVICE)
    BOARD              = $(GOWIN9K_BOARD)
    FAMILY             = $(GOWIN9K_FAMILY)
    OUTPUT_FILE        = $(GOWIN9K_BOARD).fs
    VHDL_HARDWARE_SRCS = $(VHDL_CORE_SRCS) my6502_gowintop.vhd
    TOPMODULE          = top_t65_system_gowin
    SYNTH_TYPE         = gowin
endif

ifeq ($(PLATFORM),gowin20k)
    DEVICE             = $(GOWIN20K_DEVICE)
    BOARD              = $(GOWIN20K_BOARD)
    FAMILY             = $(GOWIN20K_FAMILY)
    OUTPUT_FILE        = $(GOWIN20K_BOARD)_BE_CAREFULL_AND_CHECK_THINGS.fs
    VHDL_HARDWARE_SRCS = $(VHDL_CORE_SRCS) my6502_gowintop.vhd
    TOPMODULE          = top_t65_system_gowin
    SYNTH_TYPE         = gowin
endif



VHDL_SIM_SRCS = $(VHDL_HARDWARE_SRCS) my6502_topsim.vhd
TOP_GHDL      = top_t65_system_sim



PLATFORM_JSON  = $(OBJ_DIR)/my6502_$(PLATFORM).json

ifeq ($(V),1)
    PIPE_SYNTH =
    PIPE_PNR   =
else
    PIPE_SYNTH = > $(SYNTHLOG) 2>&1
    PIPE_PNR   = > $(PNRLOG) 2>&1
endif

# ========================================================================
# 🚨 3. GLOBAL TOP-LEVEL ROUTER
# ========================================================================
all: help

ifeq ($(MODE),sim)
all: $(TARGET_CXX)
endif
ifeq ($(MODE),ghdl)
all: ghdl_only
endif
ifeq ($(MODE),synth)
all: synth_target
endif
ifeq ($(MODE),pnr)
all: fpga_bitstream
endif

sim:   $(TARGET_CXX)
ghdl:  ghdl_only
synth: synth_target
pnr:   fpga_bitstream

help:
	@echo "=================================================================="
	@echo " ⚔️  SIMPLE_6502 MULTI-BOARD PRODUCTION & SIMULATION COCKPIT"
	@echo "=================================================================="
	@echo " Snabb-kommandon (Direkta mål):"
	@echo "   make help          : Visar den här snygga guiden."
	@echo "   make init          : Hämtar autonomously T65 från MiSTer master-branch."
	@echo "   make sim           : Bygger den snabba CXXRTL C++ simulatorn."
	@echo "   make ghdl          : Kör ren GHDL-simulation och dumpar till VCD."
	@echo "   make synth         : Isolerat syntessteg! Skapar JSON för vald PLATFORM."
	@echo "   make pnr           : Isolerat PnR-steg! Kör placering utifrån färdig JSON."
	@echo "=================================================================="

init: Oric_MiSTer
	$(Q)if [ ! -d "Oric_MiSTer" ]; then \
		echo "🌐 [GIT] Hämtar MiSTer T65 från master-branchen..."; \
		git clone --depth 1 -b master https://github.com Oric_MiSTer; \
	fi


# ------------------------------------------------------------------------
# 🏎️  PRODUKTIONS-REGLER FOR SIMULERING (CXXRTL & GHDL)
# ------------------------------------------------------------------------
$(TARGET_CXX): $(MAIN_OBJ) $(GENERATED_OBJ)
	@echo "🔗 [LINK] Svetsar samman objektfilerna till färdig simulator..."
	$(Q)$(CXX) $(CXXFLAGS) $(MAIN_OBJ) $(GENERATED_OBJ) -o $(TARGET_CXX)

$(MAIN_OBJ): main.cpp $(GENERATED_SRC)
	$(Q)mkdir -p $(OBJ_DIR)
	@echo "🖥️  [CXX] Kompilerar din cockpit (main.cpp -> main.o)..."
	$(Q)$(CXX) $(CXXFLAGS) -c main.cpp -o $(MAIN_OBJ)

$(GENERATED_OBJ): $(GENERATED_SRC)
	@echo "🖥️  [CXX] Kompilerar den tunga kiselmatrisen (Tar tid, körs sällan!)..."
	$(Q)$(CXX) $(CXXFLAGS) -c $(GENERATED_SRC) -o $(GENERATED_OBJ)

$(GENERATED_SRC): init $(VHDL_SIM_SRCS)
	$(Q)mkdir -p $(OBJ_DIR)
	@echo "🎛️  [YOSYS] Syntetiserar $(TOPMODULE) till rå C++ källkod..."
	$(Q)$(YOSYS) -m ghdl -p "ghdl $(GHDL_FLAGS) -gCLK_FREQ_HZ=20 $(VHDL_SIM_SRCS) -e $(TOPMODULE); write_cxxrtl -g4 -header $(GENERATED_SRC)" > $(OBJ_DIR)/cxxrtl_synth.log 2>&1

ghdl_only: init
	$(Q)rm -f work-*.cf
	@echo "📡 [GHDL] Analyserar och kompilerar alla filer steg för steg..."
	$(Q)for file in $(VHDL_SIM_SRCS); do $(GHDL) -a $(GHDL_FLAGS) $$file || exit 1; done
	@echo "🎛️  [GHDL] Binder samman din topsim: $(TOP_GHDL)..."
	$(Q)$(GHDL) -e $(GHDL_FLAGS) $(TOP_GHDL)
	@echo "🚀 [GHDL] Exekverar simulering och dumpar till waveform6502.vcd..."
	$(Q)$(GHDL) -r $(GHDL_FLAGS) $(TOP_GHDL) --vcd=waveform6502.vcd --stop-time=1000us

# ------------------------------------------------------------------------
# 🛠️ PRODUKTIONS-REGLER FÖR PHYSICAL SILICON
# ------------------------------------------------------------------------

synth_target: $(PLATFORM_JSON)


$(PLATFORM_JSON): $(VHDL_HARDWARE_SRCS) | init
	$(Q)mkdir -p $(OBJ_DIR)
	@echo "🎛️  [YOSYS] Syntetiserar $(TOPMODULE) för $(PLATFORM)..."
	$(Q)$(YOSYS) -m ghdl -p "ghdl $(GHDL_FLAGS) $(VHDL_HARDWARE_SRCS) -e $(TOPMODULE); synth_$(SYNTH_TYPE) -json $@" $(PIPE_SYNTH) $(PIPE_HANDLE_ERROR_SYNTH)
	$(Q)W_COUNT=$$(grep -i "warning" $(SYNTHLOG) | wc -l 2>/dev/null || echo 0); \
	    if [ $$W_COUNT -gt 0 ]; then \
	        printf "⚠️  $(CLR_YELLOW)%d warnings found$(CLR_RESET) in $(SYNTHLOG).\n" $$W_COUNT; \
	    fi


fpga_bitstream: $(PLATFORM_JSON)
ifeq ($(PLATFORM),ice40)
	@echo "🗺️  [NEXTPNR] Place & Route för iCE40..."
	$(Q)nextpnr-ice40 --$(ICE40_DEVICE) --json $(PLATFORM_JSON) --pcf $(ICE40_PCF) --asc $(OBJ_DIR)/my6502.asc \
		--package $(ICE40_PACKAGE) --pcf-allow-unconstrained \
		$(PIPE_PNR) $(PIPE_HANDLE_ERROR_ICE40)
	$(Q)icepack $(OBJ_DIR)/my6502.asc $(ICE40_DEVICE)_$(ICE40_PACKAGE).bin
	@echo "✅ ICE40 STRÖM S SÄKRAD! Fil: my6502.bin"
	$(Q)W_COUNT=$$(grep -i "warning" $(PNRLOG) | wc -l); \
	     printf "⚠️  $(CLR_YELLOW)%d warnings found$(CLR_RESET) in $(PNRLOG).\n" $$W_COUNT
	$(Q)echo "📊 === iCE40 KISEL-SAMMANFATTNING ==="
	$(Q)grep -E "Info: LUTs:|Info:    RAMs:|Info:     IOs:" $(PNRLOG) || true
	$(Q)echo "====================================="
 else
	@echo "🗺️  [NEXTPNR] Place & Route för $(PLATFORM) via Himbaechel..."
	$(Q)nextpnr-himbaechel --json $(PLATFORM_JSON) --write $(OBJ_DIR)/pnr_$(PLATFORM).json --device $(DEVICE) \
		--vopt cst=$(BOARD).cst --vopt family=$(FAMILY) \
		$(PIPE_PNR) $(PIPE_HANDLE_ERROR_GOWIN)
	$(Q)gowin_pack -d $(DEVICE) -o $(BOARD).fs $(OBJ_DIR)/pnr_$(PLATFORM).json
	@echo "✅ Gowin STRÖM S SÄKRAD! Fil: my6502.bin"
	$(Q)W_COUNT=$$(grep -i "warning" $(PNRLOG) | wc -l 2>/dev/null || echo 0); \
	    if [ $$W_COUNT -gt 0 ]; then \
	        printf "⚠️  $(CLR_YELLOW)%d warnings found$(CLR_RESET) in $(PLATFORM) PnR.\n" $$W_COUNT; \
	    fi
	@echo "📊 === $(PLATFORM) KISEL-SAMMANFATTNING ==="
	$(Q)grep -E "Info: LUTs:|Info:    RAMs:|Info:     IOs:" $(PNRLOG) 2>/dev/null || true
	@echo "====================================="
 endif



# ------------------------------------------------------------------------
# 🧹 UTILITIES
# ------------------------------------------------------------------------
clean:
	$(Q)rm -rf $(OBJ_DIR) $(TARGET_CXX) work-*.cf *.vcd my6502.bin *.fs *.log

.PHONY: all clean ghdl_only fpga_bitstream init help sim synth synth_target pnr init

test_vars:
	@echo "Vald plattform: $(PLATFORM)"
	@echo "Hittad enhet:   $(DEVICE)"
	@echo "Hittat kort:    $(BOARD)"
