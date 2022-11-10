#!/bin/sh

if [ "$#" -ne 3 ]; then
    echo "Should be three parameters to this script"
    exit 9
fi


SSID="ssid=\""$1"\""

#if grep -Fq "$SSID" /Users/pierre/Braingeneers/Picroscope-Firmwares/Linux-layer/CameraEnd-Scripts/scripts/sample_wpa_supplicant.conf

if grep -Fq "$SSID" /etc/wpa_supplicant/wpa_supplicant.conf
then
    # code if found
    echo "ssid already exists"
    exit 1
else
    # code if not found
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

fi


#cat >> /etc/wpa_supplicant/wpa_supplicant.conf << EOM

#network={
#    ssid="$1"
#    scan_ssid=1
#    key_mgmt=WPA-PSK
#    psk="$2"
#    priority=$3
#}
#EOM

#systemctl restart networking.service
