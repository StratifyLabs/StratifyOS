#!/bin/bash
TOOLSPATH=$1
HOST=$2
PROJ=$3
NAME=$4
ARCH=$5
FLOAT=$6
FOLDER=$7
TYPE=$8
LIB=lib${NAME}${TYPE}
echo "Installing include files in $TOOLSPATH/$HOST/include/$FOLDER"
mkdir -p $TOOLSPATH/$HOST/include/$FOLDER
cp -Rf ../include/* $TOOLSPATH/$HOST/include/$FOLDER
echo "Installing library file: $LIB.a in $TOOLSPATH/$HOST/lib/$ARCH/$FLOAT"
cp -f ./$LIB.a $TOOLSPATH/$HOST/lib/$ARCH/$FLOAT

exit 0