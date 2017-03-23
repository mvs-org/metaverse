![logo](https://github.com/mvs-live/metaverse/raw/master/doc/image/logo.png)
# [Metaverse (MVS)](http://mvs.live)
Metaverse is a decentralised system based on the blockchain technology, 
through which, a network of smart properties , digital identities and value intermediators are established.

Features:
- Digital asset register
- Digital asset exchange
- Digital identities
- Oralce register/data-feed

# MVS project
MVS is implemented based on [libbitcoin project](https://github.com/libbitcoin).
## toolchain requirements:
- C++ compiler support C++14 (g++ 5/LLVM 8.0.0)
- CMake 2.8

# build MVS
```bash
mkdir build && cd build
cmake ..
make -j4 && make install
```
optional:
```
make test
make doc
```

# Libraries dependencies
## install boost 1.56(or above)
```bash
sudo yum/apt-get/brew install libboost-all-dev
```
On OSX, you have to install **XcodeCommandLineTools** firstly.
Or, you can compile Boost by yourself.

## install zmq 4.2.0(or above)
server required.
```bash
wget https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz
cd libzmq
./autogen.sh && ./configure && make -j4
sudo make install && sudo ldconfig
```

## install secp256k1 
blockchain/database required.
```bash
git clone https://github.com/mvs-live/secp256k1
./autogen.sh
./configure --enable-module-recovery
make -j4 && sudo make install
```

##mvs-architecture
![mvs-architecture](https://github.com/mvs-live/metaverse/raw/master/doc/image/mvs-architecture.png)
##mvs-libraries
![mvs-libraries](https://github.com/mvs-live/metaverse/raw/master/doc/image/mvs-libraries.png)
