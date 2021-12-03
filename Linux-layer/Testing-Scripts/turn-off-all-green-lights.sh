for i in {1..4}; do for j in {1..6}; do ssh -t pi@camera$1$i$j.local "echo none | sudo tee /sys/class/leds/led0/trigger && echo 1 | sudo tee /sys/class/leds/led0/brightness"; done;done
