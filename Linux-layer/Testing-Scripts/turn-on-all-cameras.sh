for i in {1..4} ; do for j in {1..6}; do ssh pi@camera$1$i$j.local '/home/pi/RPi_Cam_Web_Interface/start.sh </dev/null >command.log 2>&1 &' ; done; done
