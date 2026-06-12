#!/bin/bash

sudo apt update

# As quick as possible ask the mail question
# TODO: automate this
sudo apt -y install mailutils

NOW=`date`


sudo DEBIAN_FRONTEND='noninteractive' apt-get -y -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold' upgrade
sudo DEBIAN_FRONTEND='noninteractive' apt-get -y -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold' dist-upgrade

sudo apt-get autoremove -y
sudo apt-get clean -y
sudo apt-get autoclean -y

sudo apt -y install curl
sudo apt -y install openssh-server net-tools

# take a backup before yosys & chums
# begin to dilly-dally here

cd

#only do this once after full install
if [ -e /home/home_pi.tar ]; then
    echo "Backup already taken, skipping"
else
    echo "Taking initial backup of /home/pi ..."
    cd /home
    tar -cvf /tmp/home_pi.tar pi
    sudo cp /tmp/home_pi.tar /home
    cd
fi

# Fix for appimages, salae logic probe in mind..
sudo add-apt-repository -y universe
sudo apt -y install libfuse2t64


MACH=`uname -m`
if [ $MACH == "aarch64" ]; then
    echo "Arm load for saleae not automated"
    sleep 1
else
    SALEAE=Download/Logic-2.4.44-linux-x64.AppImage
fi

curl "http://logic2api.saleae.com/download?os=linux&arch=arm64&release_channel=insider" -o $SALEAE

chmod a+rx $SALEAE



/home/fpga_packages.sh
/home/fpga_build_pnr.sh
/home/fpga_build_yosys.sh
/home/fpga_build_iceprogduino.sh
/home/fpga_build_openfpgaloader.sh


cd
tar xvf /home/fpga_demo.tar

# this is very close to the end and installation
# will run all the way here without questions
# once the mailutils package at the top
# has passed

echo "Build started:"
echo "$NOW"
echo "Build finished: "
date

# If sudo times out it should suffice to re-run this
/home/fpga_sudo_install.sh
