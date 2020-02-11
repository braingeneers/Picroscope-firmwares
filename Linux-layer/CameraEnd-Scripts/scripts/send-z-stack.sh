tar -zcvf /home/pi/temp/pics.tar.gz -C /home/pi/Pictures/$1/ .
rsync -zvrW /home/pi/temp/pics.tar.gz pi@$2.local:/home/pi/Pictures/$1/$(hostname)


