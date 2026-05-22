#!/bin/bash


sudo apt -y install \
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

git clone https://github.com/YosysHQ/apicula.git

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

