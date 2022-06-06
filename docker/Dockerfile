FROM ubuntu:20.04 as buildstage

RUN echo 'APT::Install-Recommends 0;' >> /etc/apt/apt.conf.d/01norecommends \
  && echo 'APT::Install-Suggests 0;' >> /etc/apt/apt.conf.d/01norecommends \
  && apt-get update \
  && DEBIAN_FRONTEND=noninteractive apt-get install -y sudo wget curl net-tools ca-certificates unzip

RUN DEBIAN_FRONTEND=noninteractive apt-get install -y git-core cmake automake autoconf libtool build-essential pkg-config libtool apt-utils \
    mpi-default-dev libicu-dev libbz2-dev zlib1g-dev openssl libssl-dev libgmp-dev \
  && rm -rf /var/lib/apt/lists/*

COPY . /tmp/metaverse/

ENV IS_TRAVIS_LINUX 1

RUN cd /tmp/metaverse && /bin/bash install_dependencies.sh --build-boost --build-upnpc

RUN cd /tmp/metaverse \
  && mkdir -p build && cd build && cmake .. && make -j4

FROM debian:bullseye-slim
LABEL maintainer="Team Metaverse <dev@mvs.org>" version="0.1.1" description="This is mvs-org/metaverse image" website="http://mvs.org/"
 
COPY --from=buildstage /tmp/metaverse/build/bin/mvsd /usr/local/bin/mvsd
COPY --from=buildstage /tmp/metaverse/build/bin/mvs-cli /usr/local/bin/mvs-cli

VOLUME [/root/.metaverse]

# P2P Network
EXPOSE 5251
# JSON-RPC CALL
EXPOSE 8820
# Websocket notifcations
EXPOSE 8821

CMD ["/usr/local/bin/mvsd"]
