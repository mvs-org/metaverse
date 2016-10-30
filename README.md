# mvs
metaverse temp repository

Integrated: 
- libbitcoin 
- network 
- database 
- consensus 
- blockchain
- protocol
- server

# build mvs
```bash
mkdir build && cd build
cmake ..
make 
make test
make doc
make install
```

## 开发规范:
- 注释必须符合doxygen
- 每个函数必须有单元测试代码

# build dependency
---------------------------------
## install boost
```bash
sudo yum/apt-get install boost-all-dev
```


## install zmq
### Use latest version for mvs
```bash
git clone https://github.com/zeromq/libzmq
cd libzmq
./autogen.sh && ./configure && make -j 4
make install && sudo ldconfig
```
### This version(zero-mq 4.1.5) not match mvs - not use
```bash
apt-get install libsodium18 libsodium-dev
git clone https://github.com/jedisct1/libsodium
wget https://github.com/zeromq/zeromq4-1/releases/download/v4.1.5/zeromq-4.1.5.tar.gz
./autogen.sh
./configure
make -j4
make install
sudo ldconfig
```

## install secp256k1
```bash
git clone https://github.com/bitcoin-core/secp256k1
./autogen.sh
./configure --enable-module-recovery
make && sudo make install
```
