#!/bin/sh

SSID="$(echo $1 | cut -d ':' -f 2)"
PASSWORD="$(echo $1 | cut -d ':' -f 3)"
PRIORITY="$(grep priority= < /etc/wpa_supplicant/wpa_supplicant.conf | grep -o '.$' | tail -1)"

if [ -z "$PRIORITY" ]
then
	PRIORITY=1
else
	PRIORITY=$((PRIORITY+1))
fi

cat >> /etc/wpa_supplicant/wpa_supplicant.conf << EOM

network={
    ssid="$SSID"
    psk="$PASSWORD"
    priority=$PRIORITY
}
EOM

systemctl restart networking.service
