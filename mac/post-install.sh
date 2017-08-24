#!/bin/bash
# uninstall any ancient version
test -f /usr/local/libexec/uninstall-metaverse.sh && /usr/local/libexec/uninstall-metaverse.sh || true
killall -9 mvsd && sleep 5
su $USER -c "open /Applications/Metaverse\ Metaverse.app"
sleep 5
su $USER -c "open http://127.0.0.1:8820/"
exit 0
