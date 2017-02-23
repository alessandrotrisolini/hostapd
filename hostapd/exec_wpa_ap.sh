#!/bin/bash

gnome-terminal -x sh -c "cd ../wpa_supplicant && sudo ip netns exec ns1 ./wpa_supplicant -K -t -i veth1 -D macsec_linux -c /etc/wpa_supplicant/wpa_ap.conf -dd"
