#!/bin/bash

if [ -e FPGA-HDMI ]; then
   echo "FPGA-HMDI proj already checked out.."
else
    echo "Checking out FPGA-HMDI proj .."
    git clone https://github.com/WangXuan95/FPGA-HDMI.git
fi


# copy up what we actually can use
cp FPGA-HDMI/RTL/*.v .
rm -f fpga_top.v
cp FPGA-HDMI/fpga_example/RTL/*.v .

# make the pll's
gowin_pll -i 27 -o 125 -d GW2AR-LV18QN88C8/I7 -m pll125 > pllhdmi_gen.v
gowin_pll -i 27 -o  26 -d GW2AR-LV18QN88C8/I7 -m pllpix > pllpix_gen.v



FILES="hdmi_tddr.v hdmi_tx_top.v \
       hdmi_async_fifo.v hdmi_tmds_encode.v \
       pixel_generate.v div5.v fpga_top_pll.v \
       pllhdmi_gen.v pllpix_gen.v \
       /usr/local/share/yosys/gowin/cells_sim.v"

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
