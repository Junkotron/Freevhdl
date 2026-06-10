#!/bin/bash

#run this verbatim from desktop_fpga/
LIST="1_install.sh fpga_build_pnr.sh fpga_build_yosys.sh fpga_packages.sh fpga_build_iceprogduino.sh fpga_sudo_install.sh fpga_sudo_uninstall.sh start_salae.sh fpga_build_openfpgaloader.sh"

LIST_DEMO="fpga_demo"

sudo cp $LIST /home
pushd /home
sudo chmod a+r $LIST
sudo chmod a+rx 1_install.sh
sudo mkdir fpga_demo
popd


sudo tar -cf /home/fpga_demo.tar $LIST_DEMO


pushd /home
sudo chmod a+r $LIST
sudo chmod a+rx *.sh
popd
