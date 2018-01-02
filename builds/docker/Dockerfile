FROM ubuntu:16.04
#LABEL maintainer="Jeremy Lan <air.petrichor@gmail.com>" version="0.1.1" \
#  description="This is mvs-org/metaverse image" website="http://mvs.org/" \
#  , etc..."

RUN echo 'APT::Install-Recommends 0;' >> /etc/apt/apt.conf.d/01norecommends \
  && echo 'APT::Install-Suggests 0;' >> /etc/apt/apt.conf.d/01norecommends \
  && apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y sudo wget net-tools ca-certificates unzip

RUN apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y git-core automake autoconf libtool build-essential pkg-config libtool apt-utils \
    mpi-default-dev libicu-dev libbz2-dev zlib1g-dev openssl libssl-dev libgmp-dev \
  && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y libboost-all-dev

RUN cd /tmp && wget https://cmake.org/files/v3.9/cmake-3.9.0-Linux-x86_64.sh \
  && mkdir /opt/cmake && chmod +x /tmp/cmake-3.9.0-Linux-x86_64.sh \
  && sh /tmp/cmake-3.9.0-Linux-x86_64.sh --prefix=/opt/cmake --skip-license \
  && ln -s /opt/cmake/bin/cmake /usr/local/bin

RUN cd /tmp && git clone https://github.com/mvs-org/secp256k1 \
  && cd secp256k1 \
  && ./autogen.sh && ./configure --enable-module-recovery --without-bignum && make -j4 && make install \
  && ldconfig && rm -rf /tmp/secp256k1*

RUN cd /tmp && wget https://github.com/zeromq/libzmq/releases/download/v4.2.1/zeromq-4.2.1.tar.gz && tar zxf zeromq-4.2.1.tar.gz \
  && cd zeromq-4.2.1 && ./autogen.sh && ./configure && make && make install && ldconfig && rm -rf /tmp/zeromq-4.2.1*

# ** Following the official master branch code takes a long time to download, depending on the network speed.

RUN mkdir -p ~/.metaverse && cd /tmp && git clone https://github.com/mvs-org/mvs-htmls.git \
  && cp -rf mvs-htmls ~/.metaverse && rm -rf mvs-htmls*

RUN cd /tmp && git clone https://github.com/mvs-org/metaverse.git \
  && cd metaverse && mkdir build && cd build && cmake .. && make && make install

RUN mkdir -p ~/.metaverse

VOLUME [~/.metaverse]

EXPOSE 8820
ENTRYPOINT ["mvsd"]
