#!/bin/bash -e
type module >& /dev/null || source /mnt/software/Modules/current/init/bash
module load git/2.8.3
module load gcc/6.4.0
module load cmake/3.7.2
module load ccache/3.4.3
module load zlib/1.2.11
module load ninja/1.7.1
module load boost/1.60
module load htslib/1.5
unset PKG_CONFIG_LIST
if [[ $USER == "bamboo" ]]; then
  export CCACHE_DIR=/mnt/secondary/Share/tmp/bamboo.mobs.ccachedir
  export CCACHE_TEMPDIR=/scratch/bamboo.ccache_tempdir
fi
export CCACHE_BASEDIR=$PWD
export VERBOSE=1

echo "# BUILD AND TEST"

echo "## Enter build directory "
if [ ! -d build ] ; then mkdir build ; fi

echo "## Build binaries"
export CXXFLAGS="-fPIC -static-libstdc++"
( cd build && rm -rf * )
( cd build && cmake -GNinja .. )
( cd build && sed -i -e 's@/-I/mnt/software@/ -I/mnt/software@g' build.ninja && ninja )

echo "## Test cram tests"
( cd build && ninja check)
