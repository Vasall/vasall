#!/usr/bin/env bash

echo "Start building project."

DIR="$( cd "$(dirname "$0")" ; pwd -P )"
OBJ_DIR="${DIR}/obj"
BIN_DIR="${DIR}/bin"
LIB_DIR="${DIR}/lib"

# Check if the different folders already exist,
# and if not, create them.
if [ ! -d "${OBJ_DIR}" ]; then
	echo "Create OBJ-dir."
	mkdir -p "${OBJ_DIR}";
fi

if [ ! -d "${BIN_DIR}" ]; then
	echo "Create BIN-dir."
	mkdir -p "${BIN_DIR}";
fi

if [ ! -d "${LIB_DIR}" ]; then
	echo "Create LIB-dir."
	mkdir -p "${LIB_DIR}";
fi

# Check if the XSDL-library has already been cloned
if [ ! -d "${LIB_DIR}/XSDL" ]; then
	echo "XSDL-lib missing."	
	git clone https://github.com/enudstudios/XSDL.git ./lib/XSDL
fi

echo "Compile files."

echo "Finished building."
