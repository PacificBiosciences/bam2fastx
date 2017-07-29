#!/bin/bash

echo "# DEPENDENCIES"
echo "## Load modules"
source /mnt/software/Modules/current/init/bash
module load git gcc/5.3.0 cmake ccache zlib/1.2.5 ninja boost htslib

echo "## Clean"
git clean -fd

echo "## Fetch submodules"
git submodule update --init --remote

echo "# BUILD AND TEST"

echo "## Enter build directory "
if [ ! -d build ] ; then mkdir build ; fi

echo "## Build binaries"
( cd build && rm -rf * )
( cd build && cmake -DZLIB_INCLUDE_DIR=/mnt/software/z/zlib/1.2.5/include -DZLIB_LIBRARY=/mnt/software/z/zlib/1.2.5/lib/libz.so -GNinja .. )
( cd build && ninja htslibSrc )
( cd build && ninja )

echo "## Test cram tests"
( cd build && ninja check)
