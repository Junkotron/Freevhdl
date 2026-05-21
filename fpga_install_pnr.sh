#!/bin/bash


sudo apt install \
     cmake \
     libboost-filesystem-dev \
     libboost-thread-dev \
     libboost-dev \
     libboost-program-options-dev \
     libboost-iostreams-dev \
     libboost-python-dev \
     libeigen3-dev \
     libftdi1-dev \
     python3-pip \
     python3-msgpack \
     pybind11-dev \
     python3-msgspec

git clone https://github.com/YosysHQ/icestorm.git
cd icestorm
make
make install
cd


# Default is to old
# Goin' loco down in Apyculco..
pip install apycula --break-system-packages

git clone https://github.com/YosysHQ/nextpnr.git

cd nextpnr
git submodule update --init --recursive
cd

cd nextpnr
mkdir -p build && cd build
cmake .. -DARCH=ice40
make -j$(nproc)
sudo make install
cd

cd nextpnr
mkdir -p build && cd build
cmake .. -DARCH="himbaechel" -DHIMBAECHEL_UARCH="gowin"
make -j$(nproc)
sudo make install
cd


nextpnr-ice40 --hx8k --json Nand_from_vhdl.json --pcf nandy.pcf --asc nandy_vhdl.asc

nextpnr-ice40 --hx8k --json Nand_from_verilog.json --pcf nandy_verilog.pcf --asc nandy_verilog.asc

icepack nandy_vhdl.asc nandy_vhdl.bin
icepack nandy_verilog.asc nandy_verilog.bin

