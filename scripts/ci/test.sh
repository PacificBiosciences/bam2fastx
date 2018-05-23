#!/usr/bin/env bash
set -vex

########
# TEST #
########

ninja -C "${CURRENT_BUILD_DIR:-build}" -v test
