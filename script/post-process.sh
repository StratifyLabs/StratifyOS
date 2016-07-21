#!/bin/bash
TOOLSPATH=$1
HOST=$2
PROJ=$3
NAME=$4
ARCH=$5
FLOAT=$6
TYPE=$7

LIB=lib${PROJ}_${NAME}${TYPE}
echo "Installing include files in $TOOLSPATH/$HOST/include"

rsync -r --exclude=posix ../include/* $TOOLSPATH/$HOST/include
if [ "$NAME" != "link" ]; then
	mkdir -p $TOOLSPATH/$HOST/lib/ldscripts
	cp -Rf ../ldscript/* $TOOLSPATH/$HOST/lib/ldscripts
	cp -Rf ../include/posix/* $TOOLSPATH/$HOST/include
fi

if [ "$NAME" == "link" ]; then
	cp -Rf ../src/link/Link.h $TOOLSPATH/$HOST/include
	mkdir -p $TOOLSPATH/$HOST/include/posix
	mkdir -p $TOOLSPATH/$HOST/lib/ldscripts
	cp -Rf ../ldscript/*.sh $TOOLSPATH/$HOST/lib/ldscripts
	cp -Rf ../include/posix/* $TOOLSPATH/$HOST/include/posix
fi

echo "Installing library file: $LIB.a in $TOOLSPATH/$HOST/lib/$ARCH/$FLOAT"
cp -f ./$LIB.a $TOOLSPATH/$HOST/lib/$ARCH/$FLOAT


exit 0