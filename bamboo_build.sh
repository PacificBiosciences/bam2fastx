#!/bin/bash -evx
type module >& /dev/null || source /mnt/software/Modules/current/init/bash
module load git
module load gcc
module load cmake
module load ccache
module load zlib
module load ninja
module load boost
module load htslib
unset PKG_CONFIG_LIST
if [[ $USER == "bamboo" ]]; then
  export CCACHE_DIR=/mnt/secondary/Share/tmp/bamboo.mobs.ccachedir
  export CCACHE_TEMPDIR=/scratch/bamboo.ccache_tempdir
fi
export CCACHE_COMPILERCHECK='%compiler% -dumpversion'
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
