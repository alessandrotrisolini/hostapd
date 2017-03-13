# hostapd
This custom version of **hostapd** introduces the automatic establishment of *MACsec channels* between a supplicant and an authenticator.

After a successful 802.1X EAP-TLS authentication, the authenticator, which is also the Key Server, initiates a Key Agreement session in order to distribute the Security Association Key (SAK) that will be used to authenticate all the frames exchanged between itself and the supplicant.

The **hostapd** daemon is intended to manage an **Open vSwitch** by the addition/deletion of pysical ports to it. This enables to use a general purpose x86 machine -- with multiple NICs -- as a MACsec-capable switch.

## Installation

### Dependencies
Latest version of libnl (https://github.com/thom311/libnl/) is needed in order to communicate via netlink with the MACsec driver.

```bash
git clone https://github.com/thom311/libnl/
sudo apt-get install dh-autoreconf

cd libnl
./autogen.sh
./configure --prefix=/opt/libnl --disable-static
make
sudo make install

cd /etc/ld.so.conf.d
sudo echo "/opt/libnl/lib" > libnl.conf
sudo ldconfig
```

### Compile hostapd/wpa_supplicant
Starting from the root directory of this repository:

* for hostapd:
```bash
cd hostapd
make
```

* for wpa_supplicant:
```bash
cd wpa_supplicant
make
```
