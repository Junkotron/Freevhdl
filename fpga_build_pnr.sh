#!/bin/bash



git clone https://github.com/YosysHQ/icestorm.git
cd icestorm
make
cd


# Default is to old
# Goin' loco down in Apyculco..
pip install apycula --break-system-packages

git clone https://github.com/YosysHQ/nextpnr.git

git clone https://github.com/YosysHQ/apicula.git

cd nextpnr
git submodule update --init --recursive
cd

cd nextpnr
mkdir -p build_ice40 && cd build_ice40
cmake .. -DARCH=ice40
make -j$(nproc)
cd

cd nextpnr
mkdir -p build_gowin && cd build_gowin
cmake .. -DARCH="himbaechel" -DHIMBAECHEL_UARCH="gowin"
make -j$(nproc)
cd

