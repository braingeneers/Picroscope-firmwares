if [ "$(hostname)" = "$(cat /boot/hostname.txt)" ]; then
	echo "the names match" 
else
	sudo sh /home/pi/scripts/change-hostname.sh $(cat /boot/hostname.txt) 
fi
