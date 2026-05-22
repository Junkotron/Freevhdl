#!/bin/bash

#run this verbatim from desktop_fpga
LIST="1_install.sh fpga_install.sh fpga_install_pnr.sh"
LIST_DEMO="Nand.v Nand.vhd mk_from_verilog.sh mk_from_vhdl.sh mk_nand.sh nandy_verilog.pcf nandy_vhdl.pcf verilog_Nand.cst vhdl_Nand.cst"

sudo cp $LIST /home
pushd /home
sudo chmod a+r $LIST
sudo chmod a+rx 1_install.sh
sudo mkdir fpga_demo
popd

cd fpga_demo

sudo tar -cf /home/fpga_demo.tar $LIST_DEMO


pushd /home
sudo chmod a+r $LIST
sudo chmod a+rx fpga_install.sh fpga_install_pnr.sh
popd
