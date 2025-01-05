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

## Enhancements in branch ``jumperSwitchModes``
Added a 3-way-switch to select between Circle/Arrow/Scissors mode. Designed for spare ESP unit(s), in case one breaks before a gig. The broken unit, which is fixed to one of the modes, can then be swapped out for a spare unit, which is then set to the appropriate mode. 

This makeover will probably be applied to the default units as well, but maybe with a hard-wired mode selection instead of a switch.

```
Valid values defined in enum Mode:
1 = Access Point (192.168.1.22) + Circle (C)
2 = Client 1 (192.168.1.31) Arrow (A)
3 = Client 2 (192.168.1.32) Laser + Scissors (L)
```


## Sources

### Wi-Fi only version
Receive data from multiple boards: https://randomnerdtutorials.com/esp-now-many-to-one-esp32/

## Installation
Copy secrets.h_template and rename to secrets.h, fill in SSID and Password for ESP32 Wi-Fi.

## Libraries
Developed and tested with
* FastLED version 3.5.0
* ArtnetWifi version 1.5.1

