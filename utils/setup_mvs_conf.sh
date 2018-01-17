#!/bin/bash

MYIP=`curl ifconfig.me`
TARGET=~/.metaverse/mvs.conf

mkdir -p ~/.metaverse/

if [ ! -f ${TARGET} ]; then
    cp ../etc/mvs.conf ${TARGET}
    sed -i 's/127.0.0.1:8820/0.0.0.0:8820/g' ${TARGET}
    sed -i "/\[network\]/a self=${MYIP}:5251" ${TARGET}
fi
