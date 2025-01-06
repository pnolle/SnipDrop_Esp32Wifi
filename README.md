# SnipDrop
Code for Wi-Fi access point and clients for my LED rollup banner project 'SnipDrop'.
http://snippetupperlaser.com

## How it works
* Ableton Live (or any other DAW) sends out MIDI
* Qlc+ receives MIDI and has mappings to a bunch of functions, matrices etc.
* ESP32 #1 is Access Point, opens up Wi-Fi ``SnipDrop``
* Laptop with Ableton and Qlc+ as well as the other two ESP32 are on the AP's Wi-Fi.
* Qlc+ sends out Artnet to fixed IPs of all 3 ESPs.
* ESPs use FastLED to [get alight](https://i.ytimg.com/vi/rTCefc-uuEw/maxresdefault.jpg).

```
// Configure IP addresses of the local access point
IPAddress local_IP_AP(192, 168, 1, 22); // C strip
IPAddress local_IP_C1(192, 168, 1, 31); // A strip
IPAddress local_IP_C2(192, 168, 1, 32); // L strip
```

## Sources
Receive data from multiple boards: https://randomnerdtutorials.com/esp-now-many-to-one-esp32/

## Installation
Copy secrets.h_template and rename to secrets.h, fill in SSID and Password for ESP32 Wi-Fi.

## Libraries
Developed and tested with
* FastLED version 3.5.0
* ArtnetWifi version 1.5.1

## Troubleshooting

### Wi-Fi password not accepted
Try typing it, don't copy. There might be invisible chars.

### Wi-Fi password not accepted or other spooky things
When typing the password didn't fix the issue, re-flashing the MCU with the option ``Tools / Erase All Flash Before Sketch Upload: Enabled`` in Arduino IDE helped. This will probably help for other weird issues as well, in case sth's gone sideways on the dev board.

### Client doesn't get IP address via DHCP on Mac OS
Go to ``System Settings => Wi-Fi => SnipDrop Details => TCP/IP => Renew DHCP Lease`` => Asks if unsaved changes shall be applied (even if nothing has been changed). For some reason, it doesn't renew when clicking ``Apply``, so click ``Don't Apply`` and enjoy your IP address.

Via terminal, it should work with ``sudo ipconfig set en0 DHCP`` (given that en0 is your device interface).