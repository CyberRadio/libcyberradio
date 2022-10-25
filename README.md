[![libcyberradio build](https://github.com/CyberRadio/libcyberradio/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/CyberRadio/libcyberradio/actions/workflows/c-cpp.yml)

# libcyberradio Shared Library

### Description

The libcyberradio Shared Library is a C++ library that provides core 
utility classes for many CyberRadio applications. 

On any system where the libcyberradio Shared Library package is 
installed, you can find the documentation at 
`/usr/share/doc/libcyberradio/html`.

### Dependencies

## Ubuntu 22.04

    sudo apt install -y libboost-all-dev libpcap-dev libcurl4-openssl-dev libjsoncpp-dev libvolk2-dev

### Building the Package

    mkdir build
    cd build
    cmake ../ -DCMAKE_INSTALL_PREFIX=/usr/
    make
    sudo make isntall
    sudo ldconfig


