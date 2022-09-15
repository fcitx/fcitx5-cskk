#!/bin/sh
# Run from repository root and output version from CMakeLists.txt
# Helper for release automation
set -e
cat CMakeLists.txt | sed -n -e "s/project(fcitx5-cskk VERSION \(.*\))/\1/p"