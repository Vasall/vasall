#!/usr/bin/env bash

echo "Start building project."

DIR="$( cd "$(dirname "$0")" ; pwd -P )"
OBJECTDIR="${DIR}/objects"
BUILDDIR="${DIR}/build"

echo "Check object-dir."
if [ ! -d "$OBJECTDIR" ]; then
	echo "Create object-dir."
	mkdir -p "$OBJECTDIR";
else 
	echo "Object-dir already exists."
fi

echo "Check build-dir."
if [ ! -d "$BUILDDIR" ]; then
	echo "Create build-dir."
	mkdir -p "$BUILDDIR";
else 
	echo "Build-dir already exists."
fi

echo "Compile files."
make

echo "Finished building."
