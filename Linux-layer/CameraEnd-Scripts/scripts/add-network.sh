#!/bin/sh

cat >> /etc/wpa_supplicant/wpa_supplicant.conf << EOM

network={
    ssid="$1"
    scan_ssid=1
    key_mgmt=WPA-PSK
    psk="$2"
    priority=$3
}
EOM

systemctl restart networking.service
