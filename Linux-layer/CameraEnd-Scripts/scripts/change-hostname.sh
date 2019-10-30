echo "current: $(hostname) new: $1"
sudo sed -i "s/$(hostname)/$1/g" /etc/hosts
sudo sed -i "s/$(hostname)/$1/g" /etc/hostname
sudo reboot
