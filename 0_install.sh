#!/bin/bash

#run this verbatim from desktop_fpga
LIST="1_install.sh"
sudo cp $LIST /home
pushd /home
sudo chmod a+r $LIST
sudo chmod a+rx 1_install.sh
sudo mkdir fpga_demo
popd

cd fpga_demo

sudo cp $LIST_DEMO /home/fpga_demo

pushd /home/fpga_demo
sudo chmod a+r $LIST_DEMO
sudo chmod a+rx fpga_install.sh fpga_install_pnr.sh
popd
