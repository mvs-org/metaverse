# The Metaverse(mvs)
**Now switch to a private repository to develop. It will be merged when main-net go-live.**

Metaverse is a decentralised system based on the blockchain technology, 
through which, a network of smart properties , digital identities and value intermediators are established.

Features:
- Digital asset register
- Digital asset exchange
- Digital identities
- Oralce register/data-feed

# mvs project
mvs is implemented based on [libbitcoin project](https://github.com/libbitcoin).

Has integrated: 
- [libbitcoin](https://github.com/libbitcoin/libbitcoin)
- [libbitcoin-network](https://github.com/libbitcoin/libbitcoin-network)
- [libbitcoin-database](https://github.com/libbitcoin/libbitcoin-database)
- [libbitcoin-consensus](https://github.com/libbitcoin/libbitcoin-consensus)
- [libbitcoin-blockchain](https://github.com/libbitcoin/libbitcoin-blockchain)
- [libbitcoin-protocol](https://github.com/libbitcoin/libbitcoin-protocol)
- [libbitcoin-server](https://github.com/libbitcoin/libbitcoin-server)
- [libbitcoin-client](https://github.com/libbitcoin/libbitcoin-client)
- [libbitcoin-explorer](https://github.com/libbitcoin/libbitcoin-explorer)

# build mvs
```bash
mkdir build && cd build
cmake ..
make 
make test
make doc
make install
```

# build dependency
## build tools
C++ compiler support C++11
cmake 3.0

## install boost 1.56(or above)
```bash
sudo yum/apt-get install libboost-all-dev
```

## install zmq for server
```bash
git clone https://github.com/the-metaverse/libzmq
cd libzmq
./autogen.sh && ./configure && make -j 4
make install && sudo ldconfig
```
comments: zero-mq 4.2.0 or above required

## install secp256k1 for blockchain/database
```bash
git clone https://github.com/the-metaverse/secp256k1
./autogen.sh
./configure --enable-module-recovery
make -j4 && sudo make install
```
