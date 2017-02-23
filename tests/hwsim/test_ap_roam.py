# Roaming tests
# Copyright (c) 2013, Jouni Malinen <j@w1.fi>
#
# This software may be distributed under the terms of the BSD license.
# See README for more details.

from remotehost import remote_compatible
import time
import logging
logger = logging.getLogger()

import hwsim_utils
import hostapd

@remote_compatible
def test_ap_roam_open(dev, apdev):
    """Roam between two open APs"""
    hapd0 = hostapd.add_ap(apdev[0], { "ssid": "test-open" })
    dev[0].connect("test-open", key_mgmt="NONE")
    hwsim_utils.test_connectivity(dev[0], hapd0)
    hapd1 = hostapd.add_ap(apdev[1], { "ssid": "test-open" })
    dev[0].scan(type="ONLY")
    dev[0].roam(apdev[1]['bssid'])
    hwsim_utils.test_connectivity(dev[0], hapd1)
    dev[0].roam(apdev[0]['bssid'])
    hwsim_utils.test_connectivity(dev[0], hapd0)

@remote_compatible
def test_ap_roam_open_failed(dev, apdev):
    """Roam failure due to rejected authentication"""
    hapd0 = hostapd.add_ap(apdev[0], { "ssid": "test-open" })
    dev[0].connect("test-open", key_mgmt="NONE", scan_freq="2412")
    hwsim_utils.test_connectivity(dev[0], hapd0)
    params = { "ssid": "test-open", "max_num_sta" : "0" }
    hapd1 = hostapd.add_ap(apdev[1], params)
    bssid = hapd1.own_addr()

    dev[0].scan_for_bss(bssid, freq=2412)
    dev[0].dump_monitor()
    if "OK" not in dev[0].request("ROAM " + bssid):
        raise Exception("ROAM failed")

    ev = dev[0].wait_event(["CTRL-EVENT-AUTH-REJECT"], 1)
    if not ev:
        raise Exception("CTRL-EVENT-AUTH-REJECT was not seen")

    dev[0].wait_connected(timeout=5)
    hwsim_utils.test_connectivity(dev[0], hapd0)

@remote_compatible
def test_ap_roam_wpa2_psk(dev, apdev):
    """Roam between two WPA2-PSK APs"""
    params = hostapd.wpa2_params(ssid="test-wpa2-psk", passphrase="12345678")
    hapd0 = hostapd.add_ap(apdev[0], params)
    dev[0].connect("test-wpa2-psk", psk="12345678")
    hwsim_utils.test_connectivity(dev[0], hapd0)
    hapd1 = hostapd.add_ap(apdev[1], params)
    dev[0].scan(type="ONLY")
    dev[0].roam(apdev[1]['bssid'])
    hwsim_utils.test_connectivity(dev[0], hapd1)
    dev[0].roam(apdev[0]['bssid'])
    hwsim_utils.test_connectivity(dev[0], hapd0)

def test_ap_roam_wpa2_psk_failed(dev, apdev, params):
    """Roam failure with WPA2-PSK AP due to wrong passphrase"""
    params = hostapd.wpa2_params(ssid="test-wpa2-psk", passphrase="12345678")
    hapd0 = hostapd.add_ap(apdev[0], params)
    id = dev[0].connect("test-wpa2-psk", psk="12345678", scan_freq="2412")
    hwsim_utils.test_connectivity(dev[0], hapd0)
    params['wpa_passphrase'] = "22345678"
    hapd1 = hostapd.add_ap(apdev[1], params)
    bssid = hapd1.own_addr()
    dev[0].scan_for_bss(bssid, freq=2412)

    dev[0].dump_monitor()
    if "OK" not in dev[0].request("ROAM " + bssid):
        raise Exception("ROAM failed")

    ev = dev[0].wait_event(["CTRL-EVENT-SSID-TEMP-DISABLED",
                            "CTRL-EVENT-CONNECTED"], 5)
    if "CTRL-EVENT-CONNECTED" in ev:
        raise Exception("Got unexpected CTRL-EVENT-CONNECTED")
    if "CTRL-EVENT-SSID-TEMP-DISABLED" not in ev:
        raise Exception("CTRL-EVENT-SSID-TEMP-DISABLED not seen")

    if "OK" not in dev[0].request("SELECT_NETWORK id=" + str(id)):
        raise Exception("SELECT_NETWORK failed")

    ev = dev[0].wait_event(["CTRL-EVENT-SSID-REENABLED"], 3)
    if not ev:
        raise Exception("CTRL-EVENT-SSID-REENABLED not seen");

    dev[0].wait_connected(timeout=5)
    hwsim_utils.test_connectivity(dev[0], hapd0)

@remote_compatible
def test_ap_reassociation_to_same_bss(dev, apdev):
    """Reassociate to the same BSS"""
    hapd = hostapd.add_ap(apdev[0], { "ssid": "test-open" })
    dev[0].connect("test-open", key_mgmt="NONE")

    dev[0].request("REASSOCIATE")
    dev[0].wait_connected(timeout=10, error="Reassociation timed out")
    hwsim_utils.test_connectivity(dev[0], hapd)

    dev[0].request("REATTACH")
    dev[0].wait_connected(timeout=10, error="Reattach timed out")
    hwsim_utils.test_connectivity(dev[0], hapd)

    # Wait for previous scan results to expire to trigger new scan
    time.sleep(5)
    dev[0].request("REATTACH")
    dev[0].wait_connected(timeout=10, error="Reattach timed out")
    hwsim_utils.test_connectivity(dev[0], hapd)

@remote_compatible
def test_ap_roam_set_bssid(dev, apdev):
    """Roam control"""
    hostapd.add_ap(apdev[0], { "ssid": "test-open" })
    hostapd.add_ap(apdev[1], { "ssid": "test-open" })
    id = dev[0].connect("test-open", key_mgmt="NONE", bssid=apdev[1]['bssid'],
                        scan_freq="2412")
    if dev[0].get_status_field('bssid') != apdev[1]['bssid']:
        raise Exception("Unexpected BSS")
    # for now, these are just verifying that the code path to indicate
    # within-ESS roaming changes can be executed; the actual results of those
    # operations are not currently verified (that would require a test driver
    # that does BSS selection)
    dev[0].set_network(id, "bssid", "")
    dev[0].set_network(id, "bssid", apdev[0]['bssid'])
    dev[0].set_network(id, "bssid", apdev[1]['bssid'])

@remote_compatible
def test_ap_roam_wpa2_psk_race(dev, apdev):
    """Roam between two WPA2-PSK APs and try to hit a disconnection race"""
    params = hostapd.wpa2_params(ssid="test-wpa2-psk", passphrase="12345678")
    hapd0 = hostapd.add_ap(apdev[0], params)
    dev[0].connect("test-wpa2-psk", psk="12345678", scan_freq="2412")
    hwsim_utils.test_connectivity(dev[0], hapd0)

    params['channel'] = '2'
    hapd1 = hostapd.add_ap(apdev[1], params)
    dev[0].scan_for_bss(apdev[1]['bssid'], freq=2417)
    dev[0].roam(apdev[1]['bssid'])
    hwsim_utils.test_connectivity(dev[0], hapd1)
    dev[0].roam(apdev[0]['bssid'])
    hwsim_utils.test_connectivity(dev[0], hapd0)
    # Wait at least two seconds to trigger the previous issue with the
    # disconnection callback.
    for i in range(3):
        time.sleep(0.8)
        hwsim_utils.test_connectivity(dev[0], hapd0)
