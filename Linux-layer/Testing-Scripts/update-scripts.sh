for i in {1..4} ; do for j in {1..6}; do scp -r ../CameraEnd-Scripts/scripts pi@camera$1$i$j.local:/home/pi/; done; done
