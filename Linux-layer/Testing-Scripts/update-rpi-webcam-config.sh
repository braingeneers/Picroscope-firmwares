for i in {1..4} ; do for j in {1..6}; do rsync -v ./RPi_Cam_Interface_Config/raspimjpeg pi@camera$1$i$j.local:/etc/raspimjpeg; done; done
