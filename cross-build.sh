# cross comiple for arm using Docker Late Lee<li@latelee.org>
# 
# docker >= 17.05.0-ce
# host kernel >= 4.8
# TODO: build arm & x86 in one script file

#!/bin/bash
set -e

ARCH=arm64v8 #arm32v7

main(){
if [ `whoami` != "root" ];then
    echo "please using root"
    exit
fi

#if [[ $# != 1 ]]; then
#    usage
#    exit
#fi

echo "begin at"
date 

docker_prepare arm64

docker_build

echo "finish at"
date
}

usage() {
    echo "Usage:"
    echo "$0 arm | arm64"
}

myexit() {
    echo "error exit at:"
    date
    exit 0
}

prepare_qemu(){
    sudo apt-get install qemu-user-static
}

docker_prepare(){
    arch=$@

    if [[ $arch == arm ]]; then
        ARCH=arm32v7
    elif [[ $arch == arm64 ]]; then
        ARCH=arm64v8
    else
        echo "$arch not support" 
        exit
    fi
    
    echo "build arch: " $ARCH
}

docker_build(){
    if [[ ! -f /usr/bin/qemu-aarch64-static ]]; then
    echo "install qemu..."
    apt-get install qemu-user-static -y
    fi

    echo "setting env..."
    
    docker run --rm --privileged multiarch/qemu-user-static --reset -p yes || myexit
    echo "building..."
    docker build -t metaverse -f Dockerfile.cross --build-arg BUILD_FROM=$ARCH .  || myexit
    
    echo "copying..."
    docker run -d --rm --name foobar metaverse || myexit
    rm -rf output && mkdir -p output
    docker cp foobar:/usr/local/lib output
    docker cp foobar:/usr/local/bin output
    docker rm -f foobar
    
    echo "Done"
}

main $@
