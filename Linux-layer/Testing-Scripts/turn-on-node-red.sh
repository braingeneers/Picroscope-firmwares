for i in {1..3}; do for j in {2..5}; do ssh -t pi@cameraC$i$j.local "sudo systemctl enable nodered.service && sudo reboot"; done; done
