#!/bin/bash


cd ghdl
sudo make uninstall
cd

cd yosys
sudo make uninstall
cd


cd icestorm
sudo make uninstall
cd

sudo rm -f /usr/local/bin/nextpnr*
