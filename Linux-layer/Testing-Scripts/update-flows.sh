for i in {1..4} ; do for j in {1..6}; do scp ../CameraEnd-Node-Red/flows.json pi@camera$i$j.local:/home/pi/.node-red/flows.json; done; done
