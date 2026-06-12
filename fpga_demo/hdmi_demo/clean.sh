#!/bin/bash

mkdir -p old 

#FILES="fpga_top.v hdmi_tddr.v hdmi_tx_top.v \
#       hdmi_async_fifo.v hdmi_tmds_encode.v \
#       pixel_generate.v"

#mv $FILES "#*" svcd vcdextract *.vcd *~ old/
mv "#*" svcd vcdextract *.vcd *~ old/

mv *.vcd work-obj*.cf mk_oric.sh *.rtlil *.json *~ *.bin *.asc *.fs old/

