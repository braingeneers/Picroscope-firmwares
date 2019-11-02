for i in {1..3}; do for j in {1..6}; do scp flows.json pi@camera$i$j.local:/home/pi/.node-red/flows.json; done; done
