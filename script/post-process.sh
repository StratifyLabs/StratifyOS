#!/bin/bash
TOOLSPATH=$1
HOST=$2
PROJ=$3
NAME=$4
ARCH=$5
FLOAT=$6
TYPE=$7

LIB=lib${PROJ}_${NAME}${TYPE}
export SUDO_ASKPASS=${PROJ}/script/ask-pass.sh
echo "Installing include files in $TOOLSPATH/$HOST/include"
sudo mkdir -p $TOOLSPATH/$HOST/include/hwpl
sudo mkdir -p $TOOLSPATH/$HOST/include/hwdl
sudo mkdir -p $TOOLSPATH/$HOST/include/dev
sudo mkdir -p $TOOLSPATH/$HOST/lib/ldscripts

sudo rsync -r --exclude=posix ../include/* $TOOLSPATH/$HOST/include
#sudo cp -Rf ../include/* $TOOLSPATH/$HOST/include
if [ "$NAME" != "link" ]; then
	sudo cp -Rf ../ldscript/* $TOOLSPATH/$HOST/lib/ldscripts
	sudo cp -Rf ../include/posix/* $TOOLSPATH/$HOST/include
fi

if [ "$NAME" == "link" ]; then
	sudo cp -Rf ../src/link/Link.h $TOOLSPATH/$HOST/include/iface
fi

echo "Installing library file: $LIB.a in $TOOLSPATH/$HOST/lib/$ARCH/$FLOAT"
sudo cp -f ./$LIB.a $TOOLSPATH/$HOST/lib/$ARCH/$FLOAT


exit 0