#!/bin/bash -e
type module >& /dev/null || source /mnt/software/Modules/current/init/bash
module load git/2.8.3
module load gcc/4.9.2
module load cmake/3.7.2
module load ccache/3.2.3
module load zlib/1.2.8
module load ninja/1.7.1
module load boost/1.60
unset PKG_CONFIG_LIST
export CCACHE_BASEDIR=$PWD
export VERBOSE=1

echo "# BUILD AND TEST"

echo "## Enter build directory "
if [ ! -d build ] ; then mkdir build ; fi

echo "## Build binaries"
export CXXFLAGS="-fPIC -static-libstdc++"
( cd build && rm -rf * )
( cd build && cmake -DZLIB_INCLUDE_DIR=`pkg-config --cflags-only-I zlib|sed -e 's/-I//'` -DZLIB_LIBRARY=`pkg-config --libs-only-L zlib|sed -e 's/-L//;s/  *$//g'`/libz.so -GNinja .. )
( cd build && ninja htslibSrc )
( cd build && ninja )

echo "## Test cram tests"
( cd build && ninja check)
