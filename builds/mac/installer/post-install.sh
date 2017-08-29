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
su $USER
WORKDIR=`dirname $0`
FRONT=mvs-htmls
MVSBIN=bin
FTARGET=$WORKDIR/mvs-pkg/$FRONT
BTARGET=$WORKDIR/mvs-pkg/$MVSBIN
if [ -d "$FTARGET" ]; then
    echo "=> copy $FRONT to ~/Library/Application\ Support/Metaverse"
else
    echo "$FTARGET not found"
fi
su $USER -c "mkdir -p ~/Library/Application\ Support/Metaverse"
/bin/rm -rf ~/Library/Application\ Support/Metaverse/mvs-htmls
/bin/cp -rf $FTARGET ~/Library/Application\ Support/Metaverse

if [ -d "$BTARGET" ]; then
    echo "=> copy bin files to /usr/local/bin"
else
    echo "$BTARGET not found"
fi
/bin/cp -rf $BTARGET/* /usr/local/bin 
echo "=> Installed successfully."

su $USER -c "open /Applications/Metaverse.app"
sleep 15
su $USER -c "open http://127.0.0.1:8820/"
exit 0
