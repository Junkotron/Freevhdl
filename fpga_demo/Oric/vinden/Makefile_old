
clean:
	./clean.sh

all:
	./checkout_and_build.sh all all

sim:
	./checkout_and_build.sh sim all

GEN_DIR=./Oric_MiSTer/rtl
GEN_BASE=$(GEN_DIR)/oricatmos_sim
GEN_CPP=$(GEN_BASE).cpp
GEN_H=$(GEN_BASE).h
GEN_OBJ=$(GEN_BASE).o
GEN_BIN=$(GEN_DIR)/sim_oricatmos

FLAGS=-O3 -std=c++11

#FLAGS=-g -std=c++11


$(GEN_OBJ): $(GEN_CPP) $(GEN_H)
	clang++ $(FLAGS) \
 -I `yosys-config --datdir`/include/backends/cxxrtl/runtime \
 -I . \
 -c $(GEN_CPP) \
 -o $(GEN_OBJ)

runs: $(GEN_BIN)

$(GEN_BIN): $(GEN_OBJ) sim_main_vhdl.cpp frame_grab.cpp
	clang++ $(FLAGS) \
 -I `yosys-config --datdir`/include/backends/cxxrtl/runtime \
 -I $(GEN_DIR) \
 $(GEN_OBJ) sim_main_vhdl.cpp frame_grab.cpp \
 -o $(GEN_BIN)

