for i in {1..4}; do for j in {1..6}; do ssh -t pi@cameraC$i$j "echo EveeHub | sudo tee /boot/hub-hostname.txt"; done; done
