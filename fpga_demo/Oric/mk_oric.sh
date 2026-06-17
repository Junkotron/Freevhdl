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

#ORIC="reset_logic.vhd oricatmostop_sim.vhd oricatmos.vhd divn.vhd",
ORIC="reset_logic.vhd divn.vhd m6522.vhd ula.vhd pravetz8d_fdc.vhd microdisc_dummy.vhd oricatmos.vhd oricatmostop_ice40.vhd oricatmostop_gowin.vhd oricatmostop_sim.vhd"


for i in $ORIC; do
    ghdl -a -fexplicit -fsynopsys $FLAGS $i
done

if [ $MODE == "ghdl" ]; then
    exit 0
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

    yosys -m ghdl -p "ghdl $FLAGS oricatmostop_$PLATTFORM; write_rtlil oricatmostop_$PLATTFORM.rtlil"

    # This is kind of useful to check that our desired signal is in the
    # portlist
    
#    echo "tcl..."
#    read

    # Skip for now for gowin pin names does not work :(
    
    if [ $PLATTFORM == "ice40" ]; then
	yosys < yo_$PLATTFORM.txt
    else
	cp oricatmostop_$PLATTFORM.rtlil modified_$PLATTFORM.rtlil
    fi

#    echo "tcl done"
#    read
    
    VLOG_COMMON="modified_$PLATTFORM.rtlil keyboard_$PLATTFORM.rtlil"
    if [ $PLATTFORM == "ice40" ]; then
	VLOG_PLATTFORM="tristate_ice40.rtlil"
    fi

    if [ $PLATTFORM == "gowin" ]; then
	# TODO
	VLOG_PLATTFORM=""
    fi

    if [ $PLATTFORM == "sim" ]; then
	# TODO
	VLOG_PLATTFORM="floppy_track_dummy_sim.rtlil psg_sim.rtlil joystick_sim.rtlil"
    fi

    if [ $PLATTFORM == "sim" ]; then

#	yosys -p "read_rtlil write_rtlil oricatmostop_sim_verilog.rtlil"

	      # write_cxxrtl -g4 -noflatten -header oricatmos_sim.cpp \
	yosys -p " \
	      read_rtlil $VLOG_COMMON $VLOG_PLATTFORM; \
	      hierarchy -top oricatmostop_sim; \
	      proc; \
	      write_cxxrtl -g4 -header oricatmos_sim.cpp \
              "
	clang++ -g -std=c++11 \
		-I $(yosys-config --datdir)/include/backends/cxxrtl/runtime \
		-I . \
		oricatmos_sim.cpp sim_main_vhdl.cpp \
		-o sim_oricatmos
	
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

    DEVICE="GW2A-LV18PG256C7/I6"
    
    BOARD=oricatmos
    
    PNRJSON=pnr_oricatmos.json
    
    nextpnr-himbaechel --json oricatmostop_gowin.json \
                       --write $PNRJSON \
                       --device $DEVICE \
                       --vopt cst=$BOARD.cst \
		       --vopt family=GW2A-18


    gowin_pack -d $DEVICE -o oricatmos.fs $PNRJSON 
fi
