#!/bin/bash

if [[ "$SUDO_USER" == "" ]] ; then
	echo "This script requires elevated privileges."
	sudo $0
	exit;
fi

PLIST=~/Library/LaunchAgents/org.mvs.plist
if [ -f "$PLIST" ]; then
    su $SUDO_USER -c "launchctl stop mvsd"
    su $SUDO_USER -c "launchctl unload $PLIST"
fi
rm -f /usr/local/bin/mvsd /usr/local/bin/mvs-cli /usr/local/bin/uninstall-metaverse.sh $PLIST

