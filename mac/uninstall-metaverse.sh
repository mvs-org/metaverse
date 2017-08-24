#!/bin/bash

if [[ "$SUDO_USER" == "" ]] ; then
	echo "This script requires elevated privileges."
	sudo $0
	exit;
fi

PLIST=~/Library/LaunchAgents/org.mvs.metaverse.plist
su $SUDO_USER -c "launchctl stop org.mvs.metaverse"
su $SUDO_USER -c "launchctl unload $PLIST"
rm -f /usr/local/libexec/mvsd /usr/local/libexec/uninstall-metaverse.sh /usr/local/bin/ethstore $PLIST

