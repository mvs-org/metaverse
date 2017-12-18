<p align="center">
  <a href="https://www.myetpwallet.com/">
    <img src="https://mvs.org/images/metaverselogo.png" alt="">
  </a>
</p>

Metaverse Core Integration/staging Tree
=========================
[![Build Status](https://travis-ci.org/mvs-org/metaverse.svg?branch=develop)](https://travis-ci.org/mvs-org/metaverse)

# Introduction
Metaverse(MVS) is a decentralised system based on the blockchain technology, through which, a network of smart properties, digital identities and value intermediators are established.

**Metaverse on Blockchain Development Path**:

![dev-path](https://github.com/mvs-org/metaverse/raw/master/doc/image/dev-path.jpg)

**Metaverse Features**:
- [Digital Assets Register/Transfering](http://docs.mvs.org/whitepaper/index.html)
- [Digital Identity](http://docs.mvs.org/whitepaper/digital-identity.html)
- Decentralized Exchange
- Oralces and Offchain Data-feed

# MVS Project
MVS is implemented based on [libbitcoin project](https://github.com/libbitcoin).

Further Read: [Documents](http://docs.mvs.org)

# build MVS

## Compiler requirements
| Compilier | Minimum Version |  
| ---------| ---------------- | 
| gcc/g++ |   5.0             |  
| clang++ |   3.4 (8.0.0)     |  
| MSVC    |   19.0 (VS2015)   |  

C++ compiler support [C++14](http://en.cppreference.com/w/cpp/compiler_support). 
Using `c++ -v` to check c++ version.
- [Simple guide to upgrade GCC](http://docs.mvs.org/helpdoc/upgrade-gcc.html).
- [Upgrade guide for Debian/ubuntuu](https://github.com/libbitcoin/libbitcoin#debianubuntu)
- [Upgrade guide for OSX](https://github.com/libbitcoin/libbitcoin#macintosh)
- [Upgrade guide for windows](https://github.com/libbitcoin/libbitcoin#windows)

Dependencies of MVS are **static linked** (including libstdc++). 
Thus, there is no extra dependency after compilation.
Recommands Ubuntu 16.04/CentOS 7.2/Visual Studio 2015 to develop/debug/build MVS.

## Toolchain requirements
- cmake 3.0+
- git
- automake (speck256k1/ZeroMQ required)

```bash
$ yum/brew/apt-get install git cmake
$ yum/brew/apt-get install autoconf automake libtool pkg-config
```

## Build MVS
```bash
$ git clone https://github.com/mvs-org/metaverse.git
$ cd metaverse && mkdir build && cd build
$ cmake ..
$ make -j4
$ make install
```

optional:
```bash
$ make test
$ make doc
```
*Needs to configure Library Dependencies firstly.*

# Library Dependencies

Installing by bash script (sudo required).
```bash
$ ./install_dependencies.sh
```

## boost 1.56+
```bash
$ sudo yum/brew/apt-get install libboost-all-dev
```
If build boost manually, please download tar ball [HERE](http://downloads.sourceforge.net/project/boost/boost/1.58.0/boost_1_58_0.tar.bz2).

Odder than v0.7.3 code && boost 1.59/1.6x: [issue on json_parser 'placeholders::_1'](https://github.com/mvs-org/metaverse/issues/216)

## ZeroMQ 4.2.1+
Modules server/explorer required.

```bash
$ wget https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz
$ tar -xzvf zeromq-4.2.1.tar.gz
$ cd zeromq-4.2.1
$ ./autogen.sh
$ ./configure
$ make -j4
$ sudo make install && sudo ldconfig
```

## secp256k1 
Modules blockchain/database required.

```bash
$ git clone https://github.com/mvs-live/secp256k1
$ cd secp256k1
$ ./autogen.sh
$ ./configure --enable-module-recovery
$ make -j4
$ sudo make install && sudo ldconfig
```
