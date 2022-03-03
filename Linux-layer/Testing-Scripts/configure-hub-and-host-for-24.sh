#!/bin/bash
for i in {1..4}; do
	for j in {1..6}; do
		echo camera$1$i$j > /Volumes/boot/hostname.txt
		echo $2 > /Volumes/boot/hub-hostname.txt
		diskutil eject /Volumes/boot
		echo "camera$1$i$j Hubname: $2"
		echo "Load next SD card and Press any key to continue"
		while [ true ] ; do
			read -t 3 -n 1
			if [ $? = 0 ] ; then
				break ; 
			else
				echo "waiting for the keypress"
			fi
		done
	done
done

