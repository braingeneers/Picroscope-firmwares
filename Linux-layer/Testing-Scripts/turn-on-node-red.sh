for i in {1..4}; do for j in {1..6}; do ssh -t pi@cameraC$i$j.local "sudo systemctl enable nodered.service && sudo reboot"; done; done
