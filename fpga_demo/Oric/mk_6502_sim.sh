#!/bin/bash



# Used for all platforms

# use for old 0.37 ghdl
FLAGS="-fexplicit --ieee=synopsys"

rm -f work-obj93.cf

T65="T65_Pack.vhd T65_ALU.vhd T65_MCode.vhd T65.vhd"

for i in $T65; do
    ghdl -a $FLAGS T65/$i
done



COMPUTER="reset_logic.vhd divn.vhd my6502.vhd my6502_topsim.vhd"


for i in $COMPUTER; do
    ghdl -a $FLAGS $i
done

# produce the executable...




ghdl -e --ieee=synopsys -fexplicit top_t65_system_sim
ghdl -r --ieee=synopsys -fexplicit top_t65_system_sim --vcd=waveform6502.vcd --stop-time=1000us



# todo make a yosys sim version here below using .rtlil
