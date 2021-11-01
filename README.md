# Getting Started with ESP32 and Losant

Using the [ESP-IDF](https://github.com/espressif/esp-idf), connect an ESP32 to the Losant Platform, publish to the state topic, and subscribe to device commands.

## To Use

Run `idf.py menuconfig` and navigate to `Example Connection Configuration` and input your WiFi credentials.

Run `idf.py build` and confirm successful build.

Run `idf.py -p (PORT) flash monitor` which will flash the firmware to the designated port, and open the serial monitor. On Mac the command is `idf.py -p /dev/cu.SLAB_USBtoUART flash monitor`.
