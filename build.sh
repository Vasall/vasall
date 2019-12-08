#!/usr/bin/env bash

echo "Start building project."

DIR="$( cd "$(dirname "$0")" ; pwd -P )"
OBJ_DIR="${DIR}/obj"
BIN_DIR="${DIR}/bin"

echo "Check enud-lib."
 if [ ! -d "${DIR}/enud" ]; then
	echo "Failed to find enud-lib."
	echo "Run this command: git clone https://github.com/Vasall/enud.git"
	exit -1
else 
	echo "Found enud-lib."
	bash "${DIR}/enud/build.sh"
fi

echo "Check object-dir."
if [ ! -d "${OBJ_DIR}" ]; then
	echo "Create object-dir."
	mkdir -p "${OBJ_DIR}";
else 
	echo "Object-dir already exists."
fi

echo "Check binary-dir."
if [ ! -d "${BIN_DIR}" ]; then
	echo "Create binary-dir."
	mkdir -p "${BIN_DIR}";
else 
	echo "Binary-dir already exists."
fi

echo "Compile files."
make -B

echo "Finished building."
