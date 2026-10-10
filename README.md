# iButton Emulator

Wireless iButton DS1990A emulator and reader for ESP8266 with a web management interface, key database, manual read mode with cancel, Brute mode for sequential key enumeration, and a browser-based log viewer.

## Overview

The firmware turns an ESP8266 into a portable iButton emulation device. It hosts a Wi-Fi access point with a built-in web UI for managing a database of DS1990A keys, reading new keys from a 1-Wire probe, and sequentially enumerating all stored keys against a target reader. The web UI is accessible at http://192.168.4.1 once the device is powered on.

## Tested Hardware

The firmware has been built and verified in Arduino IDE 1.8.x and 2.x using ESP8266 Arduino Core 3.x. Hardware validation has been performed on the following boards:

- NodeMCU v2 with ESP-12E
- NodeMCU v3 with ESP-12E
- Wemos D1 Mini with ESP-12F

## Supported Boards

- ESP8266 family: ESP-07, ESP-12E, ESP-12F, NodeMCU v1/v2/v3, Wemos D1, Wemos D1 Mini, ESPresso Lite
- ESP32 family: compiles without modification, not tested
- RP2040 on Raspberry Pi Pico: supported by OneWireHub, not tested
- AVR Arduino Uno, Nano, Pro Mini: should compile without Wi-Fi functionality

## Pinout

| GPIO | Board Pin | Function |
|------|-----------|----------|
| GPIO2 | D4 on NodeMCU and Wemos D1 | 1-Wire data line |
| GPIO5 | D1 on NodeMCU and Wemos D1 | Tactile button to GND, uses INPUT_PULLUP |
| GND | any GND pin | Common ground with the target reader |

## Hardware Considerations

- A 4.7k pull-up resistor between GPIO2 and 3.3V is required when no external pull-up is present on the bus.
- ESP8266 GPIO is 3.3V and is not 5V tolerant. Use a series resistor or a logic level shifter if the target reader pulls DATA to 5V.
- GPIO2 is a boot strap pin. The 1-Wire pull-up keeps it HIGH during boot, so the ESP boots from flash normally.

## Installation

1. Install Arduino IDE 1.8.x or 2.x
2. Add the ESP8266 board package URL to Preferences, Additional Board Manager URLs:
   http://arduino.esp8266.com/stable/package_esp8266com_index.json
3. Install the ESP8266 by ESP8266 Community package via Boards Manager
4. Install the following libraries via Library Manager: OneWireHub, ESPAsyncTCP, ESPAsyncWebServer
5. Select the target board: NodeMCU 1.0 or Generic ESP8266 Module
6. Open ibutton_emulator.ino and upload to the device

## Usage

1. Power on the ESP. The access point "ibutton" appears within approximately one second.
2. Connect from a phone or laptop using the password "ibutton52".
3. Open a browser at http://192.168.4.1
4. Select a key from the list, click Read key to capture a new DS1990A, or click Brute to enumerate all stored keys.
5. Apply the ESP to the target reader socket. Emulation is automatic.

## Key Database

The firmware ships with 34 preloaded keys covering common universal and service codes for Metakom, Cyfral, Vizit, Eltis, and Komfort intercoms. The CRC8 of every key is recomputed at boot, so the hex codes displayed in the web UI match the bytes actually emitted on the 1-Wire bus. Custom keys captured via Read key are appended to the database and activated automatically.

## Brute Mode

The Brute button starts a sequential enumeration of every key in the database. Each key is emulated for 1.5 seconds, after which the ESP attaches the next key to the OneWireHub. After the last key, Brute mode disables itself automatically. A full pass over 34 keys takes approximately 51 seconds.

The per-key duration is controlled by the BRUTE_KEY_DURATION_MS constant at the top of the sketch. For testing against an M5StickC-based simulator, 1500 ms is sufficient. For real intercoms with slower polling, increase the value to 2000 or 2500 ms.

While Brute is active, the Stop button is available in the web UI. The physical button on the ESP and the Read key button are both disabled. Emulation itself is never interrupted.

## Web API

| Method | Endpoint | Description |
|--------|----------|-------------|
| GET | / | Main UI |
| GET | /log | Log viewer page |
| GET | /logdata | Plain text logs |
| GET | /current | State as JSON |
| GET | /setkey?idx=N | Select key by index |
| GET | /readkey | Enter read mode |
| GET | /cancelread | Exit read mode |
| GET | /startbrute | Start sequential enumeration |
| GET | /stopbrute | Stop enumeration |
| GET | /addpending | Add captured key to database and activate |
| GET | /discardpending | Discard captured key |

## Disclaimer

This project is intended for educational and authorized testing purposes only. The author does not condone or take responsibility for any unauthorized access to intercom systems or property. Users are solely responsible for compliance with all applicable laws.

## License

Released under the MIT License. See the LICENSE file for details.

## Author

whoamicrash
