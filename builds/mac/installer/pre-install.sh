#!/bin/bash - 
#===============================================================================
#
#          FILE: pre-install.sh
# 
#         USAGE: ./pre-install.sh 
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

if [[ "$SUDO_USER" == "" ]] ; then
	echo "This script requires elevated privileges."
	sudo $0
	exit;
fi

# uninstall any ancient version
test -f /usr/local/bin/uninstall-metaverse.sh && /usr/local/bin/uninstall-metaverse.sh || true
killall -9 mvsd && sleep 5
METAVERSE=/Applications/Metaverse.app
if [ -d "$METAVERSE" ]; then
    rm -rf /Applications/Metaverse.app
fi
