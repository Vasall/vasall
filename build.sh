#!/usr/bin/env bash

echo "Start building project."

java -jar ./XSDL/HeaderJoiner.jar -o ./XSDL/xsdl.h -t ./XSDL/headers/XSDL.h ./XSDL/headers/XSDL_node.h ./XSDL/headers/XSDL_stdnodes.h ./XSDL/headers/XSDL_context.h ./XSDL/headers/XSDL_render.h ./XSDL/headers/XSDL_text.h

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
