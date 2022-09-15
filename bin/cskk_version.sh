#!/bin/sh
# Run from repository root and output cskk version from CMakeLists.txt
# Helper for release automation
cat CMakeLists.txt | sed -n -e "s/pkg_check_modules(LIBCSKK REQUIRED IMPORTED_TARGET \"cskk>=\(.*\)\")/\1/p"