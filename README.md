# BeeHiveLive
This respository describes a bee hive monitor that is integrated to Home Assistant as a zigbee end device using ESPHome. The bee hive monitor is a custom made PCBA that includes an I2C interface for temperature/humidity measurement, load cell for weight measurement and solar charging circuitry for Lithium-Ion batteries.

## Software Development
Disclaimer: I have no idea how to programm so I basically just steal code and torture every AI known to man to get my code running.
It is the software equivalent of unsing Duck Tape in order to mount a jet engine to a Boeing 787-9 Dreamliner.

It is almost 2am at night and my brain is not properly working anymore, so I would like to apologize for this documentation. Let's see where this will get us...

### Step 1: Being a failure at using the nrf52840
I planned to use a seeed studio brakeout board based microcontroller that features Zigbee connectivity.
That enables me to connect the device to my already established Zigbee mesh.
After a short search I stumbled apon twoe different options:
1. Seeed Studio XIAO ESP32C6: https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/
2. Seeed Studio XIAO NRF52840: https://wiki.seeedstudio.com/XIAO_BLE/

After some googling "ESP32C6 vs NRF52840 current consumption" it turns out, that the NRF52840 is supposed to have a really low power comsumption, making it absolutely perfect for my application.
Even something about compatibility with ESP Home and the Arduino IDE is written in the documentation, so we know that we will have an easy time programming it (Foreshadowing can be quite obvious).

#### The limitations of ESP Home and yaml
It is possible to generate code in ESP home and it actually has such a low power consumption, that I was not even able to pick it up with my USB power meter.
However there is one problem. I want to be compatible with One-Wire devices, I2C and a HX711 weight sensor.
At first I tried implementing one of my dallas temperature sensors using the One-Wire protocoll.
I fired up the code generation in ESP Home and after exactly 93.82 seconds my professional "ESPHome-YAML-Developer" career came to an end:
<img width="2010" height="98" alt="image" src="https://github.com/user-attachments/assets/df24ccdc-0d2e-4281-a9fc-826220a76eed" />

I am not going to pretend to know what happened here, but my friend who works as a copilot for microsoft knows whats up.
There is currently no library for the usage of one wire devices with the NRF52840 chip in ESP Home.
At least it looks like smarter people than me are also aware of that problem: https://github.com/esphome/esphome/issues/16163

So let's just scrap the ESP Home approach for now and use something thats lets me use my full c-programming potential. After all, ESP Home might just be dragging me down (might include sarcasm).

#### The Savior? - An avaliable library for NRF52840 in the Arduino IDE
