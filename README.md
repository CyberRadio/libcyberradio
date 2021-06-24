# libcyberradio Shared Library

### Description

The libcyberradio Shared Library is a C++ library that provides core 
utility classes for many CyberRadio applications. 

On any system where the libcyberradio Shared Library package is 
installed, you can find the documentation at 
`/usr/share/doc/libcyberradio/html`.

### Building the Package

We have designed the layout of the source tree for the Driver to support 
the `makedeb` and `makerpm` utilities from the 
[Unified Package Repository](http://asterix.mamd.g3ti.local:81/repo), 
so we *strongly* suggest that you use those. Follow the instructions on 
the repository site to set up access on your particular system.

Debian-based systems (such as Ubuntu) use `makedeb`. The `g3-makedeb` 
package provides this utility.
> `sudo apt-get install g3-makedeb`

Red Hat-based systems (such as Fedora and CentOS) use `makerpm`. The 
`g3-makerpm` package provides this utility.
> `sudo yum install g3-makerpm`

To build the package:
* Clone this repository
* Change to the top-level source directory (the one containing the 
  `libcyberradio` directory and this README file)
* Run either `makedeb` or `makerpm` as appropriate to build the package
    * Debian
> `makedeb libcyberradio`
    * Red Hat
> `makerpm libcyberradio`
