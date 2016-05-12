#!/bin/bash

BUILD=$1

declare -a PROJECTS=('EntityFu' 'EntityFuTester')

if [ -z "$BUILD" ]; then
BUILD='debug'
fi

echo "Building $BUILD build."

cd premake

#premake4 --file=premake4.lua xcode3
#premake4 --file=premake4.lua vs2013
premake4 --file=premake4.lua gmake

cd ../build/

ruby -pi -e "gsub(/-Wl,-x/, '')" *.make

VERBOSE="verbose=0"

for i in "${PROJECTS[@]}"
do
	make $VERBOSE config=$BUILD -f "$i".make
done

cd ..
