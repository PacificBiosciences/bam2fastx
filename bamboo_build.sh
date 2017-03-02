#!/bin/bash -e
type module >& /dev/null || source /mnt/software/Modules/current/init/bash
module load git/2.8.3
module load gcc/4.9.2
module load cmake/3.7.2
module load ccache/3.2.3
module load zlib/1.2.5
module load ninja/1.7.1
module load boost/1.60

echo "# BUILD AND TEST"

echo "## Enter build directory "
if [ ! -d build ] ; then mkdir build ; fi

echo "## Build binaries"
export CXXFLAGS="-fPIC -static-libstdc++"
( cd build && rm -rf * )
( cd build && cmake -DZLIB_INCLUDE_DIR=/mnt/software/z/zlib/1.2.5/include -DZLIB_LIBRARY=/mnt/software/z/zlib/1.2.5/lib/libz.so -GNinja .. )
( cd build && ninja htslibSrc )
( cd build && ninja )

echo "## Test cram tests"
( cd build && ninja check)
