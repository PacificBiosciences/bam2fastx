#!/usr/bin/env bash
set -vex

################
# DEPENDENCIES #
################

## Load modules
type module >& /dev/null || . /mnt/software/Modules/current/init/bash

module purge

module load gcc
module load ccache

module load meson
module load ninja

module load boost
module load zlib

module load cram

case "${bamboo_planRepository_branchName}" in
  master)
    module load pbbam/master
    module load pbcopper/master
    ;;
  *)
    module load pbbam/develop
    module load pbcopper/develop
    ;;
esac


BOOST_ROOT="${BOOST_ROOT%/include}"
# unset these variables to have meson discover all
# boost-dependent variables from BOOST_ROOT alone
unset BOOST_INCLUDEDIR
unset BOOST_LIBRARYDIR

export CC="ccache gcc"
export CXX="ccache g++"
export CCACHE_BASEDIR="${PWD}"

if [[ $USER == bamboo ]]; then
  export CCACHE_DIR=/mnt/secondary/Share/tmp/bamboo.${bamboo_shortPlanKey}.ccachedir
  export CCACHE_TEMPDIR=/scratch/bamboo.ccache_tempdir
fi

# call the main build+test scripts
export CURRENT_BUILD_DIR="build"
export ENABLED_TESTS="true"

export LDFLAGS="-static-libstdc++ -static-libgcc"

bash scripts/ci/build.sh
bash scripts/ci/test.sh
