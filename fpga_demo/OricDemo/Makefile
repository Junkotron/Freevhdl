# =============================================================================
# 1. MILJÖVARIABLER OCH VERKTYGSKONVENTIONER
# =============================================================================
CXX        := clang++
CXXFLAGS   := -O3 -std=c++17
YOSYS      := yosys

# DINA ORIGINAL-GHDL-FLAGGOR
GHDL_FLAGS := -fexplicit -fsynopsys --std=08 --work=work
PLATTFORM  := sim

# Den exakta sökvägen till CXXRTL:s runtime-headers
CXXRTL_INC := $(shell $(YOSYS)-config --datdir)/include/backends/cxxrtl/runtime

# VERBOSITY CONTROL: "make sim V=1" visar råa kommandon
ifeq ($(V),1)
  Q  :=
  YQ :=
else
  Q  := @
  YQ := -q
endif

# SÖKVÄGAR
CLONE_DIR   := oric_mister_src
MY_RTL_DIR  := rtl

# EXAKTA SÖKVÄGAR TILL DIN LINJÄRA ORDNING
APPLE_FILES := $(MY_RTL_DIR)/dpram_dummy.vhd \
               $(CLONE_DIR)/rtl/apple2_disk/disk_ii_rom.vhd \
               $(CLONE_DIR)/rtl/apple2_disk/drive_ii.vhd \
               $(CLONE_DIR)/rtl/apple2_disk/disk_ii.vhd

T65_FILES   := $(addprefix $(CLONE_DIR)/rtl/T65/, T65_Pack.vhd T65_ALU.vhd T65_MCode.vhd T65.vhd)

ROM_FILES   := $(addprefix $(CLONE_DIR)/rtl/rom/, BASIC10.vhd BASIC11.vhd BASIC22.vhd DIAG10.vhd MICRODIS.vhd ORIC1SDCARD.vhd PRAVETZ8D.vhd PRAVETZ8D_FDC.vhd BASIC11A.vhdl BASIC11B.vhdl)

ORIC_FILES  := $(MY_RTL_DIR)/reset_logic.vhd \
               $(MY_RTL_DIR)/divn.vhd \
               $(MY_RTL_DIR)/microdisc_dummy.vhd \
               $(CLONE_DIR)/rtl/m6522.vhd \
               $(CLONE_DIR)/rtl/ula.vhd \
               $(CLONE_DIR)/rtl/pravetz8d_fdc.vhd \
               $(CLONE_DIR)/rtl/oricatmos.vhd \
               $(MY_RTL_DIR)/oricram.vhd \
               $(MY_RTL_DIR)/oricatmostop.vhd

# INTERMEDIÄRA FILER OCH OBJEKT
GEN_CPP    := cpu_ula_sim.cc
GEN_OBJ    := cpu_ula_sim.o
BIN_SIM    := sim_oricatmos
BIN_INJECT := sim_cpu_inject

# =============================================================================
# 2. AUTOMATISK MENYHANTERING & HUVUDMÅL
# =============================================================================
.DEFAULT_GOAL := help

help: ## Visar denna interaktiva hjälpmeny
	@echo "========================================================================"
	@echo " 🌌 ORIC ATMOS CXXRTL SIMULATOR LAB - MAKEFILE MENU"
	@echo "========================================================================"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-18s\033[0m %s\n", $$1, $$2}'

all: sim inject ## Kompilerar ALLA tillgängliga simulator-targets i labbet

sim: $(BIN_SIM) ## Standardmål: Bygger huvudsimulatorn (fullmaskinen)

inject: $(BIN_INJECT) ## Standardmål: Bygger inject maskinen

# =============================================================================
# 3. AUTOMATISK KLONING
# =============================================================================
checkout-and-clean:
	$(Q)if [ ! -d $(CLONE_DIR) ]; then \
		echo "-> Hittade inte källkoden. Klonar MiSTer-devel/Oric_MiSTer..."; \
		git clone https://github.com/MiSTer-devel/Oric_MiSTer.git $(CLONE_DIR); \
	fi
	@echo "-> Rensar dolda GHDL-cachefiler lokalt..."
	$(Q)rm -rf work-obj*.cf *.cf

# =============================================================================
# 4. DET LINJÄRA HÅRDVARUFLÖDET
# =============================================================================
$(GEN_CPP): | checkout-and-clean
	@echo "-> Lagar Microdisc-syntaxen i den klonade oricatmos.vhd via in-memory sed..."
	$(Q)sed -i 's/inst_microdisc : work.Microdisc/inst_microdisc : entity work.Microdisc/g' $(CLONE_DIR)/rtl/oricatmos.vhd
	@echo "-> Analyserar VHDL-filer i din stenhårda linjära ordning..."
	$(Q)ghdl -a $(GHDL_FLAGS) $(APPLE_FILES)
	$(Q)ghdl -a $(GHDL_FLAGS) $(T65_FILES)
	$(Q)ghdl -a $(GHDL_FLAGS) $(ROM_FILES)
	$(Q)ghdl -a $(GHDL_FLAGS) $(ORIC_FILES)
	@echo "-> Bygger dina lokala .rtlil-komponenter för Verilog-filerna..."
	$(Q)$(YOSYS) $(YQ) -p "read_verilog -sv $(CLONE_DIR)/rtl/keyboard.sv; write_rtlil keyboard.rtlil"
	$(Q)$(YOSYS) $(YQ) -p "read_verilog -sv $(MY_RTL_DIR)/floppy_track_dummy.sv; write_rtlil floppy_track_dummy.rtlil"
	$(Q)$(YOSYS) $(YQ) -p "read_verilog -sv $(CLONE_DIR)/rtl/joystick.sv; write_rtlil joystick.rtlil"
	$(Q)$(YOSYS) $(YQ) -p "read_verilog $(CLONE_DIR)/rtl/psg.v; write_rtlil psg_sim.rtlil"
	@echo "-> Exporterar toppmodulen från ditt lokala work-bibliotek..."
	$(Q)$(YOSYS) $(YQ) -m ghdl -p "ghdl $(GHDL_FLAGS) oricatmostop; write_rtlil oricatmostop.rtlil"
	@echo "-> Slår samman ALLA lokala .rtlil-block och kör write_cxxrtl..."
	$(Q)$(YOSYS) $(YQ) -p " \
		read_rtlil oricatmostop.rtlil keyboard.rtlil floppy_track_dummy.rtlil psg_sim.rtlil joystick.rtlil; \
		hierarchy -top oricatmostop; \
		proc; \
		opt -undriven -fine; \
		write_cxxrtl -noflatten -g4 -header $(GEN_CPP) \
	"
	@echo "-> [EXPERIMENT] Kör en isolerad extra export för enbart CPU..."
	$(Q)$(YOSYS) $(YQ) -p " \
		read_rtlil oricatmostop.rtlil keyboard.rtlil floppy_track_dummy.rtlil psg_sim.rtlil joystick.rtlil; \
		hierarchy -top \\t65_Brtl; \
		proc; \
		write_cxxrtl -noflatten -g4 -header cpu_only_sim.cc \
	"

# =============================================================================
# 5. SEPARAT OBJEKTKOMPILERING (Det nya hemliga vapnet!)
# =============================================================================
$(GEN_OBJ): $(GEN_CPP)
	@echo "-> Isolerar hårdvarukod: Kompilerar tung CXXRTL till statiskt objekt [$(GEN_OBJ)]..."
	$(Q)$(CXX) $(CXXFLAGS) -I$(CXXRTL_INC) -I. -c $(GEN_CPP) -o $(GEN_OBJ)

# =============================================================================
# 6. BLIXTSNABB SLUTLÄNKNING
# =============================================================================
$(BIN_SIM): $(GEN_OBJ) oric_sim_main.cpp frame_grab.cpp  snapshot_utils.cpp ## Bygger fullmaskinen mot det frysta hårdvaruobjektet
	@echo "-> Blixtlänkar din simulator [$(BIN_SIM)] mot det statiska objektet..."
	$(Q)$(CXX) $(CXXFLAGS) -I$(CXXRTL_INC) -I. $(GEN_OBJ) oric_sim_main.cpp frame_grab.cpp \
	snapshot_utils.cpp -o $(BIN_SIM)

$(BIN_INJECT): $(GEN_OBJ) oric_sim_inject.cpp snapshot_utils.cpp ## Bygger CPU-injektionstarget (skugg-ROM / registrering uppifrån)
	@echo "-> Blixtlänkar CPU-injektionstarget [$(BIN_INJECT)]..."
	$(Q)$(CXX) $(CXXFLAGS) -I$(CXXRTL_INC) -I. $(GEN_OBJ) oric_sim_inject.cpp \
	snapshot_utils.cpp retro_monitor.cpp -o $(BIN_INJECT)

# =============================================================================
# 7. REGLER FÖR SANERING (Kliniskt ren arbetsmiljö!)
# =============================================================================
cclean: ## Sanerar bara det som har att göra med dom icke-genererade C-filerna
	@echo "-> Snyggar till lite grann och slänger binärerna och körfiler..."
	$(Q)rm -f oric_registers.snapshot oric_ram.bin
	$(Q)rm -f frame_*.png frame_*.ppm
	$(Q)rm -f $(BIN_SIM) $(BIN_INJECT)


clean: cclean ## Sanerar källkodsträdet (tar bort binärer, .o, .rtlil, bilder, work-filer)
	@echo "-> Snyggar till i labbet och slänger temporärt skräp..."
	$(Q)rm -f cpu_ula_sim.cc cpu_ula_sim.h cpu_only_sim.cc cpu_only_sim.h vcpu_ula_sim.cpp Vcpu_ula_sim.h $(GEN_OBJ)
	$(Q)rm -f *.rtlil *.o
	$(Q)rm -rf work-obj*.cf work/
	@echo "🔥 Källkodsträdet är kliniskt rent!"

distclean: clean ## Totalutrensning: Återställer även klonade externa källkodsfiler
	@echo "-> Totalutrensning: Återställer nikiiv-originalfiler..."
	$(Q)if [ -d $(CLONE_DIR) ]; then \
		cd $(CLONE_DIR) && git checkout -f rtl/oricatmos.vhd 2>/dev/null || true; \
	fi
