#!/usr/bin/env bash

echo "Start building project."

java -jar ./ENUD/HeaderJoiner.jar -o ./ENUD/enud.h -t ./ENUD/headers/ENUD.h \
./ENUD/headers/ENUD_utils.h ./ENUD/headers/ENUD_text.h ./ENUD/headers/ENUD_img.h \
./ENUD/headers/ENUD_node.h ./ENUD/headers/ENUD_stdnodes.h ./ENUD/headers/ENUD_context.h \
./ENUD/headers/ENUD_render.h 

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
make -B

echo "Finished building."
