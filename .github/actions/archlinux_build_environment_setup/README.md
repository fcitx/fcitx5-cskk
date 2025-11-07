# archlinux_build_environment_setup

Installs one example environment's build dependencies for fcitx5-cskk, checking out the latest fcitx5, cskk from the github repo.
Which means it installs wayland, qt6, fcitx5, cskk, and all dependencies of them under /opt/fcitx. 
Cmake files will be at /opt/fcitx/lib/cmake/Fcitx5Core/Fcitx5CoreConfig.cmake

Depends on archlinux. Caller should use 

    runs-on: ubuntu-latest
    container: archlinux:latest

or alike to use the archlinux container.

Implicitly depends on CC and CXX environment variables.