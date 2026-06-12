#!/bin/bash


cd ghdl
sudo make uninstall
cd

# Does not have uninstall so this halfling will have to do
# todo could add more here...
cd yosys
sudo rm /usr/local/bin/yosys*
cd


cd icestorm
sudo make uninstall
cd

sudo rm -f /usr/local/bin/nextpnr*
