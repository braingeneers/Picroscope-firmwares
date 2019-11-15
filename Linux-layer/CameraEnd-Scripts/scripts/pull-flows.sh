#!/bin/sh
FLOWS=https://raw.githubusercontent.com/braingeneers/Picroscope-firmwares/master/Linux-layer/CameraEnd-Node-Red/flows.json
#until wget -r -q --spider $FLOWS;
#do
#	sleep 1
#done

wget --retry-connrefused --waitretry=1 --read-timeout=20 --timeout=15 -t 5 $FLOWS -O /home/pi/.node-red/flows.json && systemctl restart nodered.service


