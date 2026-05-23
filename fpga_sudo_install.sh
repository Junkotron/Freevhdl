#!/bin/bash


cd ghdl
sudo make install
cd

cd yosys
sudo make install
cd

cd ghdl-yosys-plugin
sudo mkdir -p /usr/local/share/yosys/plugins
sudo cp ghdl.so /usr/local/share/yosys/plugins/
cd

cd icestorm
sudo make install
cd

cd nextpnr/build_gowin
sudo make install
cd

cd nextpnr/build_ice40
sudo make install
cd

