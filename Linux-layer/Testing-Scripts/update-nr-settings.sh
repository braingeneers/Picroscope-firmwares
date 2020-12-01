for i in {1..4} ; do for j in {1..6}; do rsync -v ../CameraEnd-Node-Red/settings.js pi@camera$1$i$j.local:/home/pi/.node-red/settings.js; done; done
