for i in {1..4}; do for j in {1..6}; do ssh -t pi@camera$1$i$j.local "echo $2 | sudo tee /boot/hub-hostname.txt"; done; done
