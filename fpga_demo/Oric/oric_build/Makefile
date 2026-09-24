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
