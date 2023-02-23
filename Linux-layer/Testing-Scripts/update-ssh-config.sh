for i in {1..4} ; do for j in {1..6}; do rsync -v ./ssh_config/config pi@camera$1$i$j.local:/home/pi/.ssh/config; done; done
