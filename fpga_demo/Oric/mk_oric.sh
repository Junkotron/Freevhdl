#!/bin/bash

PLATTFORM=$1

MODE=$2


# Used for all platforms
FLAGS="-fexplicit -fsynopsys --std=08"

rm -f work-obj93.cf

APPLE="dpram_dummy.vhd disk_ii_rom.vhd drive_ii.vhd disk_ii.vhd"

for i in $APPLE; do
    ghdl -a $FLAGS apple2_disk/$i
done

T65="T65_Pack.vhd T65_ALU.vhd T65_MCode.vhd T65.vhd"

for i in $T65; do
    ghdl -a $FLAGS T65/$i
done


DISKS="BASIC10.vhd BASIC11.vhd BASIC22.vhd DIAG10.vhd MICRODIS.vhd ORIC1SDCARD.vhd PRAVETZ8D.vhd PRAVETZ8D_FDC.vhd BASIC11A.vhdl BASIC11B.vhdl"

for i in $DISKS; do
    ghdl -a $FLAGS rom/$i
done

ORIC="reset_logic.vhd divn.vhd m6522.vhd ula.vhd pravetz8d_fdc.vhd microdisc_dummy.vhd oricram.vhd oricatmos.vhd oricatmostop.vhd"

for i in $ORIC; do
#    FLIPS="-gENABLE_VIA=true -gENABLE_DISK=true -gENABLE_HID=true -gENABLE_ROM=true -gENABLE_ULA=false -gENABLE_CPU=true"
#    FLIPS="-gENABLE_VIA=false -gENABLE_DISK=false -gENABLE_HID=false -gENABLE_ROM=false -gENABLE_ULA=false -gENABLE_CPU=true"

#    ghdl -a -fexplicit -fsynopsys $FLAGS $FLIPS $i
    ghdl -a -fexplicit -fsynopsys $FLAGS $i
done


if [ $MODE == "ghdl" ]; then
    exit 0
fi

do_clang() {
	clang++ -O3 -std=c++11 \
		-I $(yosys-config --datdir)/include/backends/cxxrtl/runtime \
		-I . \
		oricatmos_sim.cpp sim_main_vhdl.cpp frame_grab.cpp \
		-o sim_oricatmos
}

do_clang_snabb() {
	clang++ -c -O3 -std=c++11 \
		-I $(yosys-config --datdir)/include/backends/cxxrtl/runtime \
		-I . \
		sim_main_vhdl.cpp 
}

if [ $PLATTFORM == "sim" ]; then

    if [ $MODE == "clang" ]; then
	
	# Just recompile c...
	do_clang

	# this is just for when fighting with getting
	# sim_main_vhdl.cpp to compile with quick turn-arounds
	# todo: actual makefile...
	#do_clang_snabb 
	exit 0
	
    fi
    
fi


if [ $MODE == "rom" ]; then
    
64tass oric_test.asm -b -o oric_test.rom
python3 mk_rom.py
mkdir -p rom
cp BASIC10.vhd rom/
    
fi


do_yosys() {

    yosys -p "read_verilog keyboard.sv; write_rtlil keyboard_$PLATTFORM.rtlil"

    if [ $PLATTFORM == "ice40" ]; then
	yosys -p "read_verilog tristate.v; write_rtlil tristate_$PLATTFORM.rtlil"
    fi

    if [ $PLATTFORM == "sim" ]; then
	yosys -p "read_verilog apple2_disk/floppy_track_dummy.sv; write_rtlil floppy_track_dummy_$PLATTFORM.rtlil"
	yosys -p "read_verilog joystick.sv; write_rtlil joystick_$PLATTFORM.rtlil"

	
	yosys -p "read_verilog psg.v; write_rtlil psg_sim.rtlil"
    fi

    yosys -m ghdl -p "ghdl $FLAGS $FLIPS oricatmostop; write_rtlil oricatmostop_$PLATTFORM.rtlil"

    
    if [ $PLATTFORM == "ice40" ]; then
	yosys < yo_$PLATTFORM.txt
    else
	cp oricatmostop_$PLATTFORM.rtlil modified_$PLATTFORM.rtlil
    fi
    
    VLOG_COMMON="modified_$PLATTFORM.rtlil keyboard_$PLATTFORM.rtlil"

    if [ $PLATTFORM == "ice40" ]; then
	VLOG_PLATTFORM="tristate_ice40.rtlil"
    fi

    if [ $PLATTFORM == "gowin" ]; then
	# TODO
	VLOG_PLATTFORM=""
    fi

    if [ $PLATTFORM == "sim" ]; then
	VLOG_PLATTFORM="floppy_track_dummy_sim.rtlil psg_sim.rtlil joystick_sim.rtlil"
    fi

    if [ $PLATTFORM == "sim" ]; then

	## OBS DENNA "GÖR" INGET ##
##	echo "issuing command: 	yosys -p \" \
##	      read_rtlil $VLOG_COMMON $VLOG_PLATTFORM; \
##	      hierarchy -top oricatmostop; \
##	      proc; \
##	      write_cxxrtl -noflatten -g4 -header oricatmos_sim.cpp \
##	      \""



	yosys -p " \
	      read_rtlil $VLOG_COMMON $VLOG_PLATTFORM; \
	      hierarchy -top oricatmostop; \
	      proc; \
	      write_cxxrtl -noflatten -g4 -header oricatmos_sim.cpp \
             "

#	yosys -p " \
#	      read_rtlil $VLOG_COMMON $VLOG_PLATTFORM; \
#	      hierarchy -top \\t65_Brtl; \
#	      proc; \
#	      write_cxxrtl -noflatten -g4 -header ../../cpu_ula_sim.cc \
#              "

#	yosys -p " \
#	      read_rtlil $VLOG_COMMON $VLOG_PLATTFORM; \
#	      hierarchy -top oricatmostop; \
#	      proc; \
#	      write_cxxrtl -noflatten -g4 -header oricatmos_sim.cpp \
#             "

#	yosys -p " \
#	      read_rtlil $VLOG_COMMON $VLOG_PLATTFORM; \
#              hierarchy -top \\oricatmos_Brtl; \
#	      proc; \
#	      write_cxxrtl -noflatten -g4 -header ../../cpu_ula_sim.cc \
#             "

	do_clang
	
    else
	yosys -p "read_rtlil $VLOG_COMMON $VLOG_PLATTFORM; synth_$PLATTFORM -json oricatmostop_$PLATTFORM.json"
    fi

}

if [ $MODE != "pnr" ]; then
    do_yosys $PLATTFORM
fi


if [ $MODE == "synth" ]; then
    exit 0
fi



if [ $PLATTFORM == "ice40" ]; then
    nextpnr-ice40 --hx8k --json oricatmostop_ice40.json --pcf oricatmos.pcf --asc oricatmos.asc --package ct256 --pcf-allow-unconstrained

    icepack oricatmos.asc oricatmos.bin
fi


if [ $PLATTFORM == "gowin" ]; then

    DEVICE="GW2AR-LV18QN88C8/I7"
    
    BOARD=oricatmos
    
    PNRJSON=pnr_oricatmos.json

    nextpnr-himbaechel --json oricatmostop_gowin.json \
                       --write $PNRJSON \
                       --device $DEVICE \
                       --vopt cst=$BOARD.cst \
		       --vopt family=GW2A-18C

    gowin_pack -d $DEVICE -o oricatmos.fs $PNRJSON 
fi

