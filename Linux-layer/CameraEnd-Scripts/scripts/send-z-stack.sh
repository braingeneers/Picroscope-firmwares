scp -r /home/pi/Pictures/$(ls /home/pi/Pictures -t | head -1) pi@microscopehub.local:/home/pi/Pictures/$(hostname)

