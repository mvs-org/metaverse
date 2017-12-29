#!/bin/bash - 
#===============================================================================
#
#          FILE: pre-install.sh
# 
#         USAGE: ./post-install.sh 
# 
#   DESCRIPTION: 
# 
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: Jeremy Lan 
#  ORGANIZATION: 
#       CREATED: 2017/08/28 17:14
#      REVISION:  ---
#===============================================================================
WORKDIR=`dirname $0`
METAVERSE="~/Library/Application\ Support/Metaverse"
HTML=mvs-htmls.tar.gz
MVSBIN=bin
HTMLTARGET=$WORKDIR/mvs-pkg/$HTML
BINTARGET=$WORKDIR/mvs-pkg/$MVSBIN

su $USER -c "mkdir -p $METAVERSE"

# copy htmls
/bin/rm -rf $METAVERSE/$HTML 
su $USER -c  "cp -rf $HTMLTARGET $METAVERSE"
su $USER -c "tar -zxvf $METAVERSE/$HTML -C $METAVERSE"
sleep 1
su $USER -c "rm -rf $METAVERSE/$HTML"

# install
/bin/cp -rf $BINTARGET/* /usr/local/bin 

# initialize
su $USER -c "mvsd -i" 
sleep 5


su $USER -c "open /Applications/Metaverse.app"
sleep 20
#su $USER -c "open http://127.0.0.1:8820/"
exit 0
