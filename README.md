# hostapd
This custom version of **hostapd** introduces the automatic establishment of *MACsec channels* between a supplicant and an authenticator.

After a successful 802.1X EAP-TLS authentication, the authenticator, which is also the Key Server, initiates a Key Agreement session in order to distribute the Security Association Key (SAK) that will be used to authenticate all the frames exchanged between itself and the supplicant.

The **hostapd** daemon is intended to manage an **Open vSwitch** by the addition/deletion of pysical ports to it. This enables to use a general purpose x86 machine -- with multiple NICs -- as a MACsec-capable switch.

## Installation
This version of **hostapd** has been tested only on Ubuntu 16.10 LTS (Linux kernel v4.8).
### Dependencies
Latest version of libnl (https://github.com/thom311/libnl/) is needed in order to communicate via netlink with the MACsec driver.

```bash
$ sudo apt-get install dh-autoreconf libssl-dev pkg-config bison flex

$ git clone https://github.com/thom311/libnl/

$ cd libnl
$ ./autogen.sh
$ ./configure --prefix=/opt/libnl --disable-static
$ make
$ sudo make install

$ echo "/opt/libnl/lib" | sudo tee /etc/ld.so.conf.d/libnl.conf
$ sudo ldconfig
$ export PKG_CONFIG_PATH=/opt/libnl/lib/pkgconfig
$ sudo cp /usr/src/linux-headers-$(uname -r)/include/uapi/linux/if_macsec.h /usr/include/linux/if_macsec.h
```

### Compile hostapd/wpa_supplicant
Starting from the root directory of this repository:

#### Compile hostapd:
```bash
$ cd hostapd
$ make
$ sudo cp hostapd /usr/local/bin/hostapd
```

#### Compile wpa_supplicant:
```bash
$ cd wpa_supplicant
$ make
$ sudo cp wpa_supplicant /usr/local/bin/wpa_supplicant
```

## Usage
### hostapd - access point
**hostapd** has to be launched on a machine that represents the access point to a network -- in our specific case, it is a switch. **hostapd** must be able to reach a RADIUS server, in order to authenticate the supplicant and create the MACsec channel by using the cryptographic material derived from the authentication.

#### Configure **hostapd**:
**hostapd** configuration is straightforward: it needs only a configuration file with key-value pairs. A commented example can be found in the *hostapd* folder of this repository.

#### Launch **hostapd**:
Launching **hostapd** requires that the MACsec kernel module is loaded. 
```bash
lsmod | grep macsec
```
If the output is an empty string, it means that the module has to be loaded into the kernel:
```bash
sudo modprobe macsec
```
Now **hostapd** can be launched by passing as parameters a configuration file and the name of the Open vSwitch that has to be managed: 
```bash
sudo ./hostapd /path/to/config/file -z $ovs-bridge-name
```

### wpa_supplicant
**wpa_supplicant** has to be launched on a node that represents an entity that wants to join a network. 

#### Configure **wpa_supplicant**:
Configuring **wpa_supplicant** is similar to configure **hostapd**: a configuration file is needed and an example can be found in the *wpa_supplicant* folder of this repository.

#### Launch **wpa_supplicant**:
TODO
