#!/usr/bin/env bash

echo "Start building project."

DIR="$( cd "$(dirname "$0")" ; pwd -P )"
OBJ_DIR="${DIR}/obj"
BIN_DIR="${DIR}/bin"
LIB_DIR="${DIR}/lib"

echo "Check OBJ-dir."
if [ ! -d "${OBJ_DIR}" ]; then
	echo "Create OBJ-dir."
	mkdir -p "${OBJ_DIR}";
else 
	echo "OBJ-dir already exists."
fi

echo "Check BIN-dir."
if [ ! -d "${BIN_DIR}" ]; then
	echo "Create BIN-dir."
	mkdir -p "${BIN_DIR}";
else 
	echo "BIN-dir already exists."
fi

echo "Check LIB-dir."
if [ ! -d "${LIB_DIR}" ]; then
	echo "Create LIB-dir."
	mkdir -p "${LIB_DIR}";
else 
	echo "LIB-dir already exists."
fi

echo "Check XSDL-lib."
if [ ! -d "${LIB_DIR}/XSDL" ]; then
	echo "Failed to find XSDL-lib."
	echo "Run this command: git clone https://github.com/enudstudios/XSDL.git"
	exit -1
else 
	echo "Found XSDL-lib."
fi

echo "Compile files."
make -B

echo "Finished building."
