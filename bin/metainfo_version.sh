#!/bin/sh
# Run from repository root and check the CMakeList.txt version exists in metainfo.
# Helper for release automation
set -e
version=`bin/version.sh`
grep $version org.fcitx.Fcitx5.Addon.Cskk.metainfo.xml.in