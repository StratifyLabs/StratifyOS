#!/bin/bash

DEST=$1
VERSION=$2

mkdir -p $DEST
mkdir -p $DEST/$VERSION

#Now the Compiler
productbuild --root /usr/local/CoActionOS /usr/local/CoActionOS $DEST/$VERSION/CoActionOS-Compiler-4.7.4-$VERSION.pkg



exit 0