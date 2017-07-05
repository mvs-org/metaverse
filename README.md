![logo](https://github.com/mvs-org/metaverse/raw/master/doc/image/logo.png)
Integration/staging tree
=========================

# Introduction
Metaverse(MVS) is a decentralised system based on the blockchain technology, 
through which, a network of smart properties , digital identities and value intermediators are established.

Features:
- Digital asset register/transfer
- Digital asset exchange
- Digital identities
- Oralce register/data-feed

Devlopment Path:

![dev-path](https://github.com/mvs-org/metaverse/raw/master/doc/image/dev-path.jpg)


# MVS Project
MVS is implemented based on [libbitcoin project](https://github.com/libbitcoin).
Further read: [Developers Document](https://github.com/mvs-org/metaverse/doc)

# build MVS
## toolchain requirements:
- C++ compiler support C++14 standard (g++ 5/LLVM 8.0.0/MSVC14)
- CMake 2.8 or above

```bash
git clone https://github.com/mvs-org/metaverse.git
cd metaverse && mkdir build && cd build
cmake ..
make -j4
make install
```
optional:
```
make test
make doc
```

# Libraries Dependencies
## install boost 1.56(or above)
```bash
sudo yum/apt-get/brew install libboost-all-dev
```
You can download boost from http://www.boost.org/ also.
For GNU toochain(automake/autoconf/libtool), required by zmq/secp256k1.

If use boost 1.59~1.63:
Found this issue:<https://github.com/boostorg/property_tree/pull/26> for compiling error.
```
/usr/local/include/boost/property_tree/json_parser/detail/parser.hpp:217:52: error: ‘_1’ was not declared in this scope
```
Please modify parser.hpp at first if boostorg does not release.

## install zmq 4.2.0(or above)
server/explorer required.
```bash
wget https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz
cd zeromq-4.2.1
./autogen.sh
./configure
make -j4
sudo make install && sudo ldconfig
```

## install secp256k1 
blockchain/database required.
```bash
git clone https://github.com/mvs-live/secp256k1
cd secp256k1
./autogen.sh
./configure --enable-module-recovery
make -j4
sudo make install && sudo ldconfig
```
