#!/bin/bash

#run this verbatim from desktop_fpga/
LIST="1_install.sh fpga_build_pnr.sh fpga_build_yosys.sh fpga_packages.sh fpga_iceprogduino.sh fpga_sudo_install.sh fpga_sudo_uninstall.sh"

LIST_DEMO="Nand.v Nand.vhd mk_from_verilog.sh mk_from_vhdl.sh mk_nand.sh nandy_verilog.pcf nandy_vhdl.pcf verilog_Nand.cst vhdl_Nand.cst ulademo"

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
sudo chmod a+rx *.sh
popd
