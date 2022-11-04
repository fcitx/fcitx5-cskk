#!/bin/sh
# Run from repository root and output cskk version from CMakeLists.txt
# Helper for release automation
set -e
cat CMakeLists.txt | sed -n -e "s/# GITHUB_ACTION_BUILD_CSKK_VERSION=\(.*\)/\1/p"