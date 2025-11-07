# archlinux_build_environment_setup

Installs one example environment's build dependencies for fcitx5-cskk. 
Which means it installs wayland, qt6, fcitx5, cskk, and all dependencies of them.

Depends on archlinux. Caller should use 

    runs-on: ubuntu-latest
    container: archlinux:latest

or alike to use the archlinux container.

Implicitly depends on CC and CXX environment variables.