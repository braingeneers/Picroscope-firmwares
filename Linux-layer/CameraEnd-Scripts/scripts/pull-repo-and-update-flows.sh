cd /home/pi/Picroscope-firmwares &&
git pull &&
cd /home/pi/.node-red &&
cmp flows.json /home/pi/Picroscope-firmwares/Linux-layer/CameraEnd-Node-Red/flows.json \
  || (echo 'The directory was modified' ; \
     cat flows.json > "backup-flows/backup-`date '+%Y-%m-%d-%H-%M-%S'`.json" && \
     echo "backed up old flows" ; \
     cp /home/pi/Picroscope-firmwares/Linux-layer/CameraEnd-Node-Red/flows.json . && \
     node-red-restart && \
     echo "done updating") 
