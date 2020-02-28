Metaverse Core Integration/staging Tree
=========================
[![Build Status](https://travis-ci.org/mvs-org/metaverse.svg?branch=master)](https://travis-ci.org/mvs-org/metaverse)
[![AGPL v3](https://img.shields.io/badge/license-AGPL%20v3-brightgreen.svg)](./LICENSE)

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

# Build MVS

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
Recommends Ubuntu 16.04/CentOS 7.2/Visual Studio 2015 to develop/debug/build MVS.

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
If you do not need UPnP support, you can use `"cmake -DUSE_UPNP=OFF .."` to disable it.

optional:
```bash
$ make test (should install boost_unit_test_framework)
$ make doc  (should install doxygen and graphviz)
```
*Needs to configure Library Dependencies firstly.*

# Library Dependencies

Installing by bash script (sudo required).
```bash
$ sudo ./install_dependencies.sh
```
By default, `./install_dependencies.sh` will install `ZeroMQ` `secp256k1`.  
You can install more by specify arguments, for example:
```bash
# --build-upnpc is needed is you want UPnP supporting.
$ sudo ./install_dependencies.sh --build-boost --build-upnpc
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
Sometimes we may meet the following compile error
```
undefined reference to '__gmpn_sub_n' ...
```
we may disable bignum in secp256k1 in this situation, use
```
$ ./configure --enable-module-recovery --with-bignum=no
```
and see more information here [#issue209](https://github.com/mvs-org/metaverse/issues/209)

## miniupnpc
Modules blockchain/network with UPnP function required.

```bash
$ wget http://miniupnp.tuxfamily.org/files/miniupnpc-2.0.tar.gz
$ tar -xzvf miniupnpc-2.0.tar.gz
$ cd miniupnpc-2.0
$ make -j4
$ sudo INSTALLPREFIX=/usr/local make install && sudo ldconfig
```

# Run MVS
After MVS is built successfully, there are two executable files in the _bin_ directory:

 - **mvsd** - server program  
   Runs a full metaverse node in the global peer-to-peer network.

 - **mvs-cli** - client program  
   Sent your request to the server, the server will process it and return response to your client.

Go to _bin_ diretory, and run the program.
More information please reference to [Command line usage](https://docs.mvs.org/docs/command-line.html) and [Configuration file](https://docs.mvs.org/docs/config-file.html).
```bash
$ cd bin
$ ./mvsd
$ ./mvs-cli $command $params $options
```

# Build/Run in docker

## Preparation
Install [Docker](https://docs.docker.com/).
```
wget qO https://get.docker.com/ | sh
```

## Build metaverse image
```
git clone https://github.com/mvs-org/metaverse.git
cd metaverse
docker build -t metaverse -f Dockerfile .
```

Where is your built image? It’s in your machine’s local Docker image registry:
```bash
docker images
```

## Run && Test

### Start docker container
```bash
docker run -p 8820:8820 metaverse
```

### Test
```bash
curl -X POST --data '{"jsonrpc":"2.0","method":"getinfo","params":[],"id":25}' http://127.0.0.1:8820/rpc/v2
```

### Execute mvs-cli commands
Run `mvs-cli` commands via `docker exec` command. Example:
```bash
docker exec metaverse mvs-cli getinfo
```

# Build for raspberry pi

Cross compile mvs using Docker in ubuntu 16.04

you need to change to **root** when executing the flowing command.

## env

check versin first, linux kerne and docker version:

```
host docker >= 17.05.0-ce
host kernel >= 4.8
```

install qemu:

```
apt-get install qemu-user-staic
```

## way1

Clone the mvs source.
then enter mvs source directory, then run:

```
// for arm 64 (eg: aarch64, armv8(and abover))
sudo bash ./cross-build.sh arm64
```

When everything ok, the binary file and .so/.a file will be found in output directory.