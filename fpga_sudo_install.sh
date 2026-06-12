#!/bin/bash


cd ghdl
sudo make install
cd

cd yosys/build
#sudo cmake --install build --strip
sudo make install
cd

# Needs to be here since it requires 
git clone https://github.com/ghdl/ghdl-yosys-plugin.git
cd ghdl-yosys-plugin/
time make
sudo mkdir -p /usr/local/share/yosys/plugins
sudo cp ghdl.so /usr/local/share/yosys/plugins/
cd

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

cd iCE40HX1K-EVB/programmer/iceprogduino
sudo make install
cd

cd openFPGALoader/build
sudo make install
cd

