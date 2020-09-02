Metaverse Core Integration/staging Tree
=========================
[![Build Status](https://travis-ci.org/mvs-org/metaverse.svg?branch=master)](https://travis-ci.org/mvs-org/metaverse)
[![AGPL v3](https://img.shields.io/badge/license-AGPL%20v3-brightgreen.svg)](./LICENSE)

![logo](https://www.myetpwallet.com/icons/metaverse.png)
# Introduction
Metaverse(MVS) is a decentralised system based on the blockchain technology, through which, a network of smart properties, digital identities and value intermediators are established.

**Metaverse Features**:
- [Digital Assets Register/Transfering](http://docs.mvs.org/whitepaper/index.html)
- [Digital Identity](http://docs.mvs.org/whitepaper/digital-identity.html)
- Oralces
- Decentralized Exchange

# Building MVS

## Compiler requirements
| Compilier | Minimum Version |
| ---------| ---------------- |
| gcc/g++ |   5.0             |
| clang++ |   3.4 (8.0.0)     |
| MSVC    |   19.0 (VS2015)   |

C++ compiler support [C++14](http://en.cppreference.com/w/cpp/compiler_support) standard or newer.
Dependencies of MVS binaris are **static linked** (including libstdc++).

## Toolchain requirements
- cmake 3.0+
- git
- automake (speck256k1/ZeroMQ)

```bash
$ yum/brew/apt-get install git cmake
$ yum/brew/apt-get install autoconf automake libtool pkg-config
```


# Setup Library Dependencies

## By install_dependencies.sh
Installing `ZeroMQ` `secp256k1` automatically by:
```bash
$ sudo ./install_dependencies.sh
```
Installing `boost` `upnp` automatically by:
```bash
$ sudo ./install_dependencies.sh --build-boost --build-upnpc
```
## Manually
### boost 1.56+
```bash
$ sudo yum/brew/apt-get install libboost-all-dev
```
[Download Boost 1.69.0](https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.gz).

## zeromq 4.2.1+
```bash
$ wget https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz
$ tar -xzvf zeromq-4.2.1.tar.gz
$ cd zeromq-4.2.1
$ ./autogen.sh
$ ./configure
$ make -j4
$ sudo make install && sudo ldconfig
```
`server/explorer` requires ZeroMQ.

## secp256k1 
```bash
$ git clone https://github.com/mvs-live/secp256k1
$ cd secp256k1
$ ./autogen.sh
$ ./configure --enable-module-recovery
$ make -j4
$ sudo make install && sudo ldconfig
```

Sometimes we may got the following compile error `undefined reference to '__gmpn_sub_n' ...`
See more details here [#issue209](https://github.com/mvs-org/metaverse/issues/209)
```
$ ./configure --enable-module-recovery --with-bignum=no
```

## miniupnpc
```bash
$ wget http://miniupnp.tuxfamily.org/files/miniupnpc-2.0.tar.gz
$ tar -xzvf miniupnpc-2.0.tar.gz
$ cd miniupnpc-2.0
$ make -j4
$ sudo INSTALLPREFIX=/usr/local make install && sudo ldconfig
```
`blockchain/network` requires upnp if specified.

# Building
```bash
$ git clone https://github.com/mvs-org/metaverse.git
$ cd metaverse && mkdir build && cd build
$ cmake -DUSE_UPNP=OFF ..
$ make -j4
$ make install
```
To enable UPnP, use `cmake ..` instead.

optional:
```bash
$ make test (should install boost_unit_test_framework)
$ make doc  (should install doxygen and graphviz)
```

# Run it
 - **mvsd** - server program  
   Runs a full metaverse node in the global peer-to-peer network.

 - **mvs-cli** - client program  
   Sent your request to the server, the server will process it and return response to your client.

```bash
$ cd bin
$ ./mvsd
$ ./mvs-cli
$ ./mvs-cli getnewaccount -h
$ ./mvs-cli $command $params $options
```
Read More [Command line usage](https://docs.mvs.org/docs/command-line.html) and [Configuration file](https://docs.mvs.org/docs/config-file.html).

# Building/Run under docker

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
apt-get install qemu-user-static
```

## way1

Clone the mvs source.
then enter mvs source directory, then run:

```
// for arm 64 (eg: aarch64, armv8(and abover))
sudo bash ./cross-build.sh arm64
```

When everything ok, the binary file and .so/.a file will be found in output directory.
