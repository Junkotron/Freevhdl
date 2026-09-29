# === Konfiguration för den Isolerade Baslinjen (Utan namn-mumbo-jumbo) ===

NIKIIV_DIR   = ./nikiiv_src
YOSYS_CONFIG = yosys-config

CXX          = clang++

# Hämta ENDAST inkluderingsflaggorna från yosys, strunta i deras debug/optimering-flaggor
YOSYS_INC    = -I/usr/local/share/yosys/include -I/usr/local/share/yosys/include/backends/cxxrtl/runtime

# Tvinga fram stenhård optimering, native arkitektur och modern C++
CXXFLAGS     = -O3 -march=native -funroll-loops -Wall -std=c++20 $(YOSYS_INC)


#CXXFLAGS     = -O3 -Wall -std=c++14 $(shell $(YOSYS_CONFIG) --cxxflags) -I/usr/local/share/yosys/include/backends/cxxrtl -I/usr/local/share/yosys/include/backends/cxxrtl/runtime


LDFLAGS      = $(shell $(YOSYS_CONFIG) --ldflags) -lstdc++

# Dina egna källfiler (bor säkert i rotmappen, skrivs ALDRIG över)
MY_DPRAM      = dpram.vhd
MY_GENRIC_RAM = bram_generic.vhd
MY_RAM        = oricram.vhd
MY_ROM        = bram_rom.vhd
MY_MICRODISC  = microdisc_dummy.vhd
MY_VHDL_1     = oricatmos.vhd
MY_TOP_VHDL   = oricatmostop.vhd
MAIN_CPP      = main.cpp

# We have hijacked BASIC10 !!
# VHDL-filer: Ändrade till nikiivs sanna, exakta filnamn på disken!
VHDL_SRCS    = $(NIKIIV_DIR)/T65_Pack.vhd \
               $(NIKIIV_DIR)/T65_ALU.vhd \
               $(NIKIIV_DIR)/T65_MCode.vhd \
               $(NIKIIV_DIR)/T65.vhd \
               $(NIKIIV_DIR)/ula.vhd \
               $(NIKIIV_DIR)/m6522.vhd \
               $(MY_DPRAM) \
               $(NIKIIV_DIR)/disk_ii_rom.vhd \
               $(NIKIIV_DIR)/drive_ii.vhd \
               $(NIKIIV_DIR)/disk_ii.vhd \
               $(NIKIIV_DIR)/BASIC11A.vhdl \
               BASIC10.vhd \
               $(NIKIIV_DIR)/PRAVETZ8D.vhd \
               $(NIKIIV_DIR)/PRAVETZ8D_FDC.vhd \
               $(MY_GENRIC_RAM) \
               $(MY_MICRODISC) \
               $(MY_RAM) \
               $(MY_ROM) \
               $(MY_VHDL_1) \
               $(MY_TOP_VHDL)

# Mellanfiler (.rtlil) – floppy_track läser din lokala dummy i roten
RTLIL_FILES  = keyboard_sim.rtlil \
               floppy_track_dummy_sim.rtlil \
               joystick_sim.rtlil \
               psg_sim.rtlil \
               oricatmostop_sim.rtlil

GEN_CXX      = oricatmos_sim.cpp
GEN_H        = oricatmos_sim.h
TARGET       = sim_oricatmos
OBJS         = main.o oricatmos_sim.o oric_rig.o dasm.o dasm_table.o util/logger.o util/argsparser.o

all: $(TARGET) emu_6502

# 🔥 AUTOMAGISK KÄLLKODSTRIGGER: Om sandlådan är tom vid en ny utcheckning, hämta källkoden först!
$(NIKIIV_DIR)/T65.vhd $(NIKIIV_DIR)/keyboard.sv $(NIKIIV_DIR)/joystick.sv $(NIKIIV_DIR)/psg.v $(NIKIIV_DIR)/ula.vhd:
	@echo "⚠️  Hittade ingen källkod i $(NIKIIV_DIR)!"
	@echo "🚀 Gör setup_sandbox.sh körbar och drar igång den..."
	@chmod +x setup_sandbox.sh
	./setup_sandbox.sh

# --- VERILOG/SYSTEMVERILOG-Mellanfiler (Nu med hårt källkods-beroende för ny utcheckning) ---
keyboard_sim.rtlil: $(NIKIIV_DIR)/keyboard.sv $(NIKIIV_DIR)/T65.vhd
	yosys -p "read_verilog -sv $<; write_rtlil $@"

floppy_track_dummy_sim.rtlil: floppy_track_dummy.sv $(NIKIIV_DIR)/T65.vhd
	yosys -p "read_verilog -sv $<; write_rtlil $@"

joystick_sim.rtlil: $(NIKIIV_DIR)/joystick.sv $(NIKIIV_DIR)/T65.vhd
	yosys -p "read_verilog -sv $<; write_rtlil $@"

psg_sim.rtlil: $(NIKIIV_DIR)/psg.v $(NIKIIV_DIR)/T65.vhd
	yosys -p "read_verilog $<; write_rtlil $@"

# --- VHDL-Mellanfil ---
oricatmostop_sim.rtlil: $(VHDL_SRCS)
	ghdl -a --std=08 -fsynopsys -fexplicit $(VHDL_SRCS)
	yosys -m ghdl -p "ghdl --std=08 -fsynopsys -fexplicit oricatmostop; write_rtlil $@"

# --- CXXRTL GENERERING ---
$(GEN_CXX): $(RTLIL_FILES)
	@echo "🛠️  Sätter ihop alla RTLIL-block..."
	yosys -p "\
		read_rtlil $(RTLIL_FILES); \
		hierarchy -top oricatmostop; \
		proc; \
		opt; \
		wreduce; \
		clean; \
		write_cxxrtl -noflatten -g4 -header $(GEN_CXX)"

# --- SEPARAT KOMPILERINGS-STEG ---
oricatmos_sim.o: $(GEN_CXX)
	$(CXX) $(CXXFLAGS) -c $(GEN_CXX) -o $@

main.o: $(MAIN_CPP) $(GEN_CXX) dasm.hpp oric_rig.hpp
	$(CXX) $(CXXFLAGS) -c $(MAIN_CPP) -o $@

oric_rig.o: oric_rig.cpp oric_rig.hpp $(GEN_CXX)
	$(CXX) $(CXXFLAGS) -c oric_rig.cpp -o $@

dasm.o: dasm.cpp dasm.hpp
	$(CXX) $(CXXFLAGS) -c dasm.cpp -o $@

dasm_table.o: dasm_table.cpp dasm.hpp
	$(CXX) $(CXXFLAGS) -c dasm_table.cpp -o $@


$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

# Normal städning: Tar bara bort det som kompilerats/genererats av denna Makefile.
# Efter en 'make clean' kan du omedelbart köra 'make' igen!
clean:
	@echo "🧹 Städar kompileringsskräp, objektfiler, mellanfiler och modeller..."
	rm -f $(GEN_CXX) $(GEN_H) $(TARGET)
	rm -f *.o
	rm -f *~ *.cf *.rtlil
	@ghdl --clean 2>/dev/null || true

distclean: clean
	@echo "🗑️  Brutaltvättar sandboxen och källkodsmapparna..."
	rm -rf $(NIKIIV_DIR)/*
	rm -rf Oric_miSTer_temp
	@echo "✨ Repot är nu kliniskt rent! Nästa 'make' hämtar kod automatiskt."


.PHONY: all run clean distclean mjukis romconv

# --- DELAD LOGGER (Används av både kislis och mjukis) ---
util/logger.o: util/logger.cpp util/logger.hpp
	$(CXX) $(CXXFLAGS) -Iutil -c util/logger.cpp -o $@

# --- MJUKIS-SIMULATOR (make mjukis) ---
MJUKIS_OBJS = util/golden_6502.o util/golden_step.o util/logger.o util/emu_main.o util/argsparser.o

util/emu_main.o: util/emu_main.cpp util/golden_6502.hpp util/logger.hpp
	$(CXX) $(CXXFLAGS) -Iutil -c util/emu_main.cpp -o $@

util/golden_6502.o: util/golden_6502.cpp util/golden_6502.hpp util/logger.hpp
	$(CXX) $(CXXFLAGS) -Iutil -c util/golden_6502.cpp -o $@

util/golden_step.o: util/golden_step.cpp util/golden_6502.hpp util/logger.hpp
	$(CXX) $(CXXFLAGS) -Iutil -c util/golden_step.cpp -o $@

emu_6502: $(MJUKIS_OBJS)
	$(CXX) $(MJUKIS_OBJS) -o util/emu_6502 $(LDFLAGS)

# --- ROM-KONVERTERARE (make romconv) ---
romconv: util/rom_parser.cpp
	$(CXX) $(CXXFLAGS) util/rom_parser.cpp -o util/rom_parser


# =====================================================================
# 🧪 MÅL FÖR HÅRDVARUTESTER (VHDL TESTBENCH)
# =====================================================================

# Verktyg för VHDL-simulering
GHDL = ghdl
GHDL_FLAGS = --std=08 # Vi använder VHDL-2008 för modern och ren kod (t.ex. to_string)

.PHONY: test test_clean

test: test_clean
	@echo "🔍 [GHDL] Analyserar VHDL-filer..."
	$(GHDL) -a $(GHDL_FLAGS) bram_generic.vhd
	$(GHDL) -a $(GHDL_FLAGS) bram_rom.vhd
	$(GHDL) -a $(GHDL_FLAGS) bram_rom_tb.vhd
	@echo "🔨 [GHDL] Elaborerar testbänk..."
	$(GHDL) -e $(GHDL_FLAGS) bram_rom_tb
	@echo "🏃 [GHDL] Kör hårdvarutest i glaskuben..."
	$(GHDL) -r $(GHDL_FLAGS) bram_rom_tb --stop-time=1us --max-stack-alloc=1024
	@echo "✨ [GHDL] Testkörning klar!"

# =====================================================================
# 🧪 MÅL FÖR HÅRDVARUTESTER (VHDL TESTBENCH)
# =====================================================================

# Verktyg för VHDL-simulering
GHDL = ghdl
GHDL_FLAGS = --std=08 # Vi använder VHDL-2008 för modern och ren kod (t.ex. to_string)

.PHONY: test test_clean test_m6522

test: test_clean
	@echo "🔍 [GHDL] Analyserar VHDL-filer..."
	$(GHDL) -a $(GHDL_FLAGS) bram_generic.vhd
	$(GHDL) -a $(GHDL_FLAGS) bram_rom.vhd
	$(GHDL) -a $(GHDL_FLAGS) bram_rom_tb.vhd
	@echo "🔨 [GHDL] Elaborerar testbänk..."
	$(GHDL) -e $(GHDL_FLAGS) bram_rom_tb
	@echo "🏃 [GHDL] Kör hårdvarutest i glaskuben..."
	$(GHDL) -r $(GHDL_FLAGS) bram_rom_tb --stop-time=1us --max-stack-alloc=1024
	@echo "✨ [GHDL] Testkörning klar!"

# Dedikerat mål för VIA 6502-testbänken
test_m6522: test_clean
	@echo "🔍 [GHDL] Analyserar VHDL-filer för M6522..."
	$(GHDL) -a $(GHDL_FLAGS) $(NIKIIV_DIR)/T65_Pack.vhd
	$(GHDL) -a $(GHDL_FLAGS) $(NIKIIV_DIR)/m6522.vhd
	$(GHDL) -a $(GHDL_FLAGS) m6522_tb.vhd
	@echo "🔨 [GHDL] Elaborerar M6522-testbänk..."
	$(GHDL) -e $(GHDL_FLAGS) m6522_tb
	@echo "🏃 [GHDL] Kör VIA-hårdvarutest..."
	$(GHDL) -r $(GHDL_FLAGS) m6522_tb --stop-time=2us --max-stack-alloc=1024
	@echo "✨ [GHDL] VIA-testkörning klar!"

test_clean:
	@echo "🧹 Rensar gamla GHDL-simuleringsfiler..."
	$(GHDL) --clean
	rm -f e~bram_rom_tb.o e~m6522_tb.o work-obj08.cf


# =====================================================================
# 🎛️ AUTOMATISERAT TESTRAMVERK FÖR 6502 ASSEMBLER (asmtest/)
# =====================================================================

ASM6502      = xa
ASMFLAGS     = -w -c

# De delade hårdvaru-objektfiler som krävs för kiselmodellen [4]
TEST_REQ_OBJS = oricatmos_sim.o oric_rig.o dasm.o dasm_table.o util/logger.o util/argsparser.o

# Hitta dynamiskt alla domar-C++-filer i mappen asmtest/ [4]
ASM_CPP_SRCS  = $(wildcard asmtest/*.cpp)
# Skapa listan över de körbara binärerna [4]
ASM_TEST_BINS = $(ASM_CPP_SRCS:.cpp=)

.PHONY: test_asm asmtest_clean

# 🎯 1. HUVUDMÅL: Beror nu direkt på att ALLA testbinärer är färskbyggda!
test_asm: $(ASM_TEST_BINS)
	@echo "🏃 Startar sekventiell exekvering av 6502-hårdvarutester..."
	@for test_exe in $(ASM_TEST_BINS); do \
		echo "📋 Kör testbinär: $$test_exe..."; \
		./$$test_exe || exit 1; \
	done
	@echo "🏆 [SUCCESS] Alla tupler i asmtest/ gick igenom kisel-verifieringen!"

# 🎯 2. STATISK MÖNSTERREGEL: Tvingar fram fullständig beroendekedja!
# Denna rad säger: För varje mål i $(ASM_TEST_BINS) (t.ex. asmtest/via_timer_test),
# så BEROR det målet stenhårt på BÅDE sin .cpp-fil OCH sin matchande .asm-fil!
$(ASM_TEST_BINS): asmtest/%: asmtest/%.cpp asmtest/%.asm $(TEST_REQ_OBJS)
	@echo "🗜️  Kompilerar 6502-assembler: asmtest/$*.asm -> asmtest/$*.bin"
	$(ASM6502) $(ASMFLAGS) asmtest/$*.asm -o asmtest/$*.bin
	@echo "🔧 Länkar fristående testdomare för $*..."
	$(CXX) $(CXXFLAGS) $< $(TEST_REQ_OBJS) -o $@ $(LDFLAGS)

# Specifik städning för testmappen [4]
asmtest_clean:
	@echo "🧹 Rensar bort kompilerade testbinärer och assembler-binfiler..."
	rm -f asmtest/*.bin
	rm -f $(ASM_TEST_BINS)
