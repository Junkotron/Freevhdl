#!/bin/bash

sudo apt update

# As quick as possible ask the mail question
# TODO: automate this
sudo apt -y install mailutils

sudo DEBIAN_FRONTEND='noninteractive' apt-get -y -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold' upgrade
sudo DEBIAN_FRONTEND='noninteractive' apt-get -y -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold' dist-upgrade

sudo apt-get autoremove -y
sudo apt-get clean -y
sudo apt-get autoclean -y

sudo apt -y install curl
sudo apt -y install openssh-server

# take a backup before yosys & chums
# begin to dilly-dally here

cd
cd /home
tar -cvf /tmp/home_pi.tar pi
sudo cp /tmp/home_pi.tar /home
cd


/home/fpga_install.sh

/home/fpga_install_pnr.sh

