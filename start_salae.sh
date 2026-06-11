#!/bin/bash

cd

MACH=`uname -m`
if [ $MACH == "aarch64" ]; then

    FILE=./Download/Logic-2.4.44-linux-arm64.AppImage

    if [ -e $FILE ]; then
	$FILE --no-sandbox
    else
	echo "Arm load for saleae not automated."
	echo "use saleae.com site for download"
	exit;
    fi
else
    
    # Assuming linux deathdefyingly
    ./Download/Logic-2.4.44-linux-x64.AppImage --no-sandbox

fi
