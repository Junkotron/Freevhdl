#!/bin/bash

if [ -e FPGA-HDMI ]; then
   echo "FPGA-HMDI proj already checked out.."
else
    echo "Checking out FPGA-HMDI proj .."
    git clone https://github.com/WangXuan95/FPGA-HDMI.git
fi


# copy up what we actually can use
#cp FPGA-HDMI/RTL/*.v .
#cp FPGA-HDMI/fpga_example/RTL/*.v .

FILES="hdmi_tddr.v hdmi_tx_top.v \
       hdmi_async_fifo.v hdmi_tmds_encode.v \
       pixel_generate.v div5.v fpga_top_nopll.v"

yosys -p "read_verilog -sv divn.v;
read_verilog $FILES;
hierarchy -check -top fpga_top;
synth_gowin -top fpga_top -json hdmi.json"

DEVICE="GW2AR-LV18QN88C8/I7"
BOARD=tang20k
FAMILY=GW2A-18C

nextpnr-himbaechel --json hdmi.json \
                   --write hdmi_pnr.json \
                   --device $DEVICE \
                   --vopt cst=$BOARD.cst \
		   --vopt family=$FAMILY		   

gowin_pack -d $FAMILY -o hdmi_$BOARD.fs hdmi_pnr.json
