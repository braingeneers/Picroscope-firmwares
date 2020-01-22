for dir in /home/pi/Pictures/*; do mkdir -- "$dir/$1"; done
for dir in /home/pi/Pictures/*; do mv $dir/*.jpg $dir/$1; done
