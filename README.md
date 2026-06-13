# BeeHiveLive
This repository describes a bee hive monitor that is integrated to Home Assistant as a zigbee end device. The bee hive monitor is a custom made PCBA that includes an I2C interface for temperature/humidity measurement, load cell for weight measurement and solar charging circuitry for Lithium-Ion batteries.
> [!Note]
> It is almost 2am at night and my brain is not properly working anymore, so I would like to apologize for this documentation.
> Also there might be some sarcasm inside this document since the whole process makes me question my sanity.
>
> Let's see where this will get us...

## Hardware Development
A few years ago I have already created a similar PCB with the difference, that I had less budget available and no idea how harsh the outside environment can be towards electronics. Before going into detail about this project, I would like to explain a little bit about my previous design. After all I was reusing a lot from that project and there are some nice mistakes to learn from.

### Previous mistakes to learn from
So back in university while I was writing on my masters thesis I had quite the need to procrastinate. So I designed my first PCB to monitor bee hives that had the following features:
1. Li-Ion battery
2. Solar charging
3. ESP32
4. Wifi connectivity
5. 3x DS18B20 Temperature sensors
6. 1x HX711 weight sensor

Without thinking about the housing or connectivity to some sensors I designed the PCB and made everything as compact as possible. Basically rotating every part on the PCB layout to enable the shortest traces and getting the board outline smaller and smaller while still fitting every component on the top side. I ended up with a PCB that was about 25mm x 50mm which included the footprint of the ESP32. It did work but when integrating everything it got chaotic:

- I had forgotten to add the HX711 weight sensor chip to the PCB so I had to wire it externally
- There were no mounting holes on the PCB, so it sat loose in the housing
- The housing was only an electrical box where I punched holes for the sensors with a screwdriver
- All sensors were connected with 2.54mm headers (typical headers for breakout boards etc.)
- The combination of headers on the sensors and holes in the housing meant, that It was not possible to disconnect the sensors from that housing without removing the female headers from their wires
- The housing was of course not waterproof

Surprisingly the device worked for 2 years until one winter water entered the housing and broke everything.
Before that I have already realized, that WIFI connectivity drains the battery fast. It was basically not possible to read values during the winter.

### Requirements/Features for this version 2.0
These requirements are a little misplaced under this chapter, because some of them also affect software. But anyways I wanted to build on what was good in the previous design and add improvements. So I had the following requirements:
1. Solar charging
   - Reuse 5W solar panel
   - Reuse of the CN3791 chip which enables charging of Li-Ion cells in a 1S-xP configuration
   - Add variable control of the MPPT-voltage
2. Li-Ion cells
   - Use Li-Ion cells I still have in a drawer
   - Reuse battery protection circuit
   - Add a bracket to directly mount the cells on the PCB
3. Zigbee connectivity (This might have been a mistake)
   - Remove or at least not use WIFI
   - Hope the Zigbee range is enough to connect ~20m to my house
   - External antenna to improve range
4. HX711 weight sensor
   - Improve on V1.0 by placing it directly on the PCB
5. DS18B20 temperature sensors
   - Reuse from V1.0
5. Connectivity on PCB level
   - Add I2C and SPI headers on the PCB for future sensors
7. Swap-able microcontrollers
   - Add header to mount microcontrollers with the 14-pin header footprint from Seeed Studio
8. Housing
   - Use of a waterproof housing
   - Use of round waterproof connectors for each individual sensor that can be unscrewed with beekeeper gloves
9. Order already assembled PCBAs
   - No more hand soldering of 0603 SMDs
   - Allows me to use even smaller ICs

### PCB design
#### Schematic
I used EasyEDA for the schematic and just made sure to follow the guideline for each IC. In addition I placed some decoupling capacitors.
The sensors are connected to the microcontroller pins according to the following mapping:
| Microcontroller I/O | Function |
| ------------------- | -------- |
| GPIO0 | HX711 SCK |
| GPIO1 | HX711 DATA |
| GPIO2 | Battery SoC |
| GPIO21 | CN3791 Charge |
| GPIO22 | I2C SDA |
| GPIO23 | I2C SCL |
| GPIO16 | CN3791 Done |
| GPIO17 | NC |
| GPIO19 | SPI SCK |
| GPIO20 | SPI MISO |
| GPIO18 | SPI MOSI |

The most time consuming part was to find components which are available for the placement service of JLCPCB. I did not put to much effort into making it look nice so please don't judge me to much. I am doing this during my vacation.

> [!Note]
> Let's play a fun little game: Can you find which GND is placed upside down?

<p align="center">
<img width="1937" height="1375" alt="image" src="https://github.com/user-attachments/assets/b272089d-4dd8-4f34-b830-ff022063332c" />
</p>

#### PCB Layout
For the layout first roughly placed all of the components into the groups they belong to. So for example everything related to solar charging together.
Then I estimated the size I need for the enclosure based on that. The goal was to have it as compact as possible. The required space was almost completely defined be the usage of two cylindrical Li-Ion cells.

> [!Note]
> I wanted to use the cylindrical cells not only because I had them sitting in my drawer for two years at the brink of being dead but also because they kind of look like explosives and that should scare of anyone trying to steal my bees.

I decided to go with this housing: [Industriegehäuse, 105 x 105 x 60,1mm, IP66/IP68, lichtgrau](https://www.reichelt.de/de/de/shop/produkt/industriegehaeuse_105_x_105_x_60_1mm_ip66_ip68_lichtgrau-340541)

Based on the technical drawing of the housing I created a board outline with 1-2mm margin and placed 4.3mm drilling holes to mount the PCB to the housing.

> [!Note]
> It might have been overkill to use __8__ mounting holes for such a small PCB. In case there is a hurricane that levels all of my bee hives, at least my PCB will still be in the same position. That's what you call safety margin.

I've placed the remaining components in areas that made sense based on connectors and space. Decoupling capacitors of course as close to the ICs supply pins.
As a last step I defined the back and the top side as ground planes and added breakaway tabs in each corner (That was easier than drawing more complex shapes with exact dimensions).

<p align="center">
<img width="500" height="500" alt="image" src="https://github.com/user-attachments/assets/b9fe85e2-b014-428f-af89-2cd177d55764" />
</p>

After two weeks of waiting I received 5 PCBAs. I chose a white silkscreen hoping it would stay cooler, since it will be sitting in the sun.

<p align="center">
<img width="500" height="490" alt="image" src="https://github.com/user-attachments/assets/4b60f29a-f0f1-4aea-b905-c12fbe09173f" />
</p>

### Housing and Connectors
As described in the PCB layout I've decided to use [this housing](https://www.reichelt.de/de/de/shop/produkt/industriegehaeuse_105_x_105_x_60_1mm_ip66_ip68_lichtgrau-340541) for my electronics. It is IP68 certified which is good for outside use, since it can even be submerged in water. That means I can even put my beehives under water and the electronics will still work. The first thing I did when the housing arrived was drilling some holes in it using a table drill.

> [!Warning]
> Make sure to fasten your workpiece when using a table drill. Plastics tend to get stuck on the drill and fly through your workshop with orbital escape velocity when they are not properly fixed in place. I made this mistake during my university days and still carry the scars to prove it. Don't safe time - Clamp down your workpiece - Don't force your drill in to quickly!

The holes are used to mount some [M12 circular connectors from amphenol](https://amphenolltw.com/product-info/M-Series/M-Series.M12.TCode/?). These offer IP67-IP69K protection and are available with different amounts of pins.
I used a 3-pin version for my solar panel connection and 4-pin versions for the weight and temperature sensors.
This also prevents plugging the solar panel into the wrong housing connector.

<p align="center">
<img width="1272" height="806" alt="image" src="https://github.com/user-attachments/assets/921445d5-5747-4c6a-9fa4-94367d17806e" />
</p>

Assembling the weight sensor connector on the load cell wire was quite tricky, since there is already a bee hive on top of it and the wire is just 1m long. So lets say the bees were quite interested in watching me putting everything together up close. They sat on my hands during the hole process and I was not able to assemble everything with my gloves on.

I h9ope thew swellinf will ncot get to bad, it is hard to type on thew keybpoard thos wayx...

> [!Note]
> This is the reason I wanted to have connectors that can be removed from my housing even with gloves on.

### PCB Testing
> [!Caution]
> In this PCB design there are two identical Li-Ion cells in parallel. When putting the cells on the PCB for the first time it is __strictly required__ to charge them to the exact same voltage level first!
>
> <p align="center">
> <img width="577" height="236" alt="image" src="https://github.com/user-attachments/assets/8cc8ede8-24df-4bec-82e4-68377e1ee77e" />
</p>

#### Solar charging
To test solar charging I connected a power supply to the solar charge input and added one Li-Ion battery to the PCB. At first I calibrated the maximum power voltage using the potentiometer on my PCB. This is possible by setting the lowest possible current limit on the power supply and then setting a voltage above the maximum power voltage of the solar panel.

<p align="center">
<img width="289" height="339" alt="image" src="https://github.com/user-attachments/assets/51b5ec13-d469-43e0-89c9-d2d0775136d0" />
</p>

In my case the maximum power voltage of the panel is 17.6V. So I set my lowest current limit of 100mA and a voltage of 20V on the power supply. Afterwards I adjusted the potentiometer on the PCB until the voltage on the power supply read 17.6V:

<p align="center">
<img width="665" height="387" alt="image" src="https://github.com/user-attachments/assets/10603696-ca2e-4059-940e-aaff076ad2eb" />
</p>

I made sure to test my PCBs in a harsh environment. If they survive my sawdust and solder covered workdesk, they will survive everything.
> [!Tip]
> If you want an excuse to buy a new power supply then just leave it next to a table saw.

Afterwards I also checked the maximum expected solar voltage without current limitation:

<p align="center">
<img width="665" height="275" alt="image" src="https://github.com/user-attachments/assets/eda0f42f-1dda-4060-8f2e-94f9aa9f1878" />
</p>

#### 3.3V supply
Before plugging in the microcontroller, I checked if the 3.3V supply works how it is supposed to:

<p align="center">
<img width="651" height="494" alt="image" src="https://github.com/user-attachments/assets/63974837-25f6-4f8b-85dd-6d96f0142891" />
</p>

#### Finalizing the hardware
As a last step I added a PCB antenna for the ESP32C6 and closed the lid. Then the hardware was ready for some software.

## Software Development
> [!Note]
> I have no idea how to program so I basically just steal code and torture every AI known to man to get my code running.
> It is the software equivalent of using Duck Tape in order to mount a jet engine to a Boeing 787-9 Dreamliner.

### Step 1: Being a failure at using the nrf52840
I planned to use a seeed studio breakout board based microcontroller that features Zigbee connectivity.
That enables me to connect the device to my already established Zigbee mesh.
After a short search I stumbled upon two different options:
1. [Seeed Studio XIAO ESP32C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/)
2. [Seeed Studio XIAO NRF52840](https://wiki.seeedstudio.com/XIAO_BLE/)

After some googling "ESP32C6 vs NRF52840 current consumption" it turns out, that the NRF52840 is supposed to have a really low power consumption, making it absolutely perfect for my application.
Even something about compatibility with ESP Home and the Arduino IDE is written in the documentation, so I knew that I will have an easy time programming it (Foreshadowing can be quite obvious).

#### The limitations of ESP Home and yaml
It is possible to generate code in ESP home and it actually has such a low power consumption, that I was not even able to pick it up with my USB power meter.
However there is one problem. I want to be compatible with One-Wire devices, I2C and an HX711 weight sensor.
At first I tried implementing one of my dallas temperature sensors using the One-Wire protocol.
I fired up the code generation in ESP Home and after exactly 93.82 seconds my professional "ESPHome-YAML-Developer" career came to an end:

<p align="center">
<img width="2010" height="98" alt="image" src="https://github.com/user-attachments/assets/df24ccdc-0d2e-4281-a9fc-826220a76eed" />
</p>

I am not going to pretend to know what happened here, but my friend who works as a copilot for microsoft knows whats up.
There is currently no library for the usage of one wire devices with the NRF52840 chip in ESP Home.
At least it looks like smarter people than me are also aware of that problem: https://github.com/esphome/esphome/issues/16163


So I just scrapped the ESP Home approach for now and used something that lets me use my full c-programming potential. After all, ESP Home might just be dragging me down (might include sarcasm).

#### The Savior? - An available library for NRF52840 in the Arduino IDE
The last time I used arduino was back in university. There was only a white background for the IDE which caused me to delete it.
A real developer needs a dark mode and lines of text flying through a terminal after all.

However since the Arduino IDE is aimed towards beginners like me I wanted to give it a try.
Indeed there is a library available to get started with my specific NRF52840 board. 
So I added the board URL to my preferences and tried out example sketches:
````
https://espressif.github.io/arduino-esp32/package_esp32_index.json
https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json
````
<p align="center">
<img width="696" height="464" alt="image" src="https://github.com/user-attachments/assets/d85e8646-102b-4b85-9245-276259d98ea5" />
</p>

> [!Important]
> This approach will not work and I was running straight against a wall here. Unfortunately my past self didn't know this yet and spent 4 hours of debugging and looking through forums before finally giving up.

It turns out that programming zigbee to the nrf52840 is not possible in the Arduino IDE. Again my copilot friend from microsoft told me this.
Apparently since some version of the nRF Connect SDK, zigbee is not supported anymore for the nrf52840 chip.
I did not even try to do the same thing in PlatformIO for Visual Studio Code, since there I was greeted with two issues:
1. For connectivity only bluetooth shows up
2. I cannot even find my Seeed Studio XIAO nrf52840 board

<p align="center">
<img width="1349" height="102" alt="image" src="https://github.com/user-attachments/assets/1d6770c4-67f7-4ed2-9a86-60dcbefad9e8" />
</p>

#### Going full professional since the Arduino IDE is for beginners
I guess after 14+ hours without sleep infront of three monitors with one of them only playing music videos, I can call myself professional developer. I somehow feel the urge to go to a TED talk and tell people about my journey.

Back to topic: So the recommended way by the people who built the nrf52840 is the usage of Visual Studio Code + nRF Connect SDK.
Awesome stuff and let me tell you: Now the real pain begins.

In fact I had to use Ibuprophen 400 just to keep myself motivated.
> [!Warning]
> This is just a joke. Don't do drugs, it will ruin your code. (Except caffeine. Do lots of caffeine. Nobody trusts a programmer that doesn't have a coffee IV in their arm)

I played around with the environment and found out the following facts:
1. Zigbee is not avaliable in every SDK version -> v2.9.2 seems to have the required libraries
2. It is extremely important where you create your project folder

When building code the automatically generated folder structure grows a lot. So large in fact that the code generation will fail due  to the amount of symbols in the filepath (now we know the maximum is 250 symbols).

3. Zigbee example projects do not allow the usage of the Seeed Studio board with its bootloader

The Seeed Studio version acts like a usb drive. When you double press reset, it shows up in the explorer and allows you to drag .uf2 files to it. These are not generated by the examples by default.
> [!Caution]
> This next step fried my bootloader and I do not have the required hardware to flash a new one.

Fortunately (I thought to myself) it is possible to build them by adding this line in the prj.conf file:
```
CONFIG_BUILD_OUTPUT_UF2=y
```
Finally after hours of debugging I got my .uf2 file. You cannot imagine how impressed I was by myself at this point.
I pulled the .uf2 into my nrf52840 and checked whether I could find it in Home Assistant as zigbee device.
After staring at the screen for about 10 minutes I tried to generate a simple blinky sketch in the same environment.
The LED stayed dark and I fell into a pit of rage.

After calming myself down by watching videos of cats taking catnip, I did some investigation.
My guess to what happened is, that I never defined how this uf2 file is supposed to be structured.
For comparison, this is how a .uf2 file is supposed to look like:

<p align="center">
<img width="551" height="27" alt="image" src="https://github.com/user-attachments/assets/5c7e2f65-90aa-47ec-8651-97c2d2a9a1bd" />
</p>

And this is what my sleep deprived brain came up with:

<p align="center">
<img width="530" height="26" alt="image" src="https://github.com/user-attachments/assets/d36a2f4b-3699-43ad-ba6a-de24c32204c3" />
</p>

I wonder what is usually stored between 0x00000 and 0x27000...

In the next few hours I tried to get the blinky sketch running on the nrf52840 using other build environments. First a proper Visual Studio Code + nRF Connect SDK environment. Afterwards just the Arduino IDE and ESPHome.
It took me quite some time to realize my problem, since the board still shows up as a USB device when I double-click the reset button. It ignores every .uf2 file I try flashing on it. I even wondered at some point if my built-in LED is just broken and so I hooked up a logic analyzer to the D0 Pin to check if it will toggle:

<p align="center">
<img width="2111" height="189" alt="image" src="https://github.com/user-attachments/assets/5737b3a4-8dfd-4967-be1f-efa5b4fd9ab9" />
</p>

Of course it doesn't. I had just bricked my microcontroller.
Impressive how far you can make it with a masters degree in electrical engineering.

Let's try to destroy the ESP32C6 next!

### Step 2: ESP32C6 is easy to program if only I knew how to do it
#### ESP Home works best for ESP devices for some reason
I've decided to go back to ESP Home and yaml files to generate my code, since bricking the nrf52840 heavily bruised my ego.
Hopefully the ESP32 is much easier to program.

I added about 4 lines regarding zigbee in my yaml...
```
esphome:
  name: beehivelive
  friendly_name: BeeHiveLive

esp32:
  board: seeed_xiao_esp32c6
  framework:
    type: esp-idf

zigbee:
  id: my_zigbee
  router: false
  power_source: BATTERY

logger:
  level: DEBUG

sensor:
  - platform: adc
    pin:
      number: 2 #P0.28
      mode: INPUT
    name: "SoC"
    accuracy_decimals: 2
    filters:
      - multiply: 1.694915254237288
      - calibrate_linear:
          method: exact
          datapoints:
          - 3.2 -> 0
          - 3.55 -> 20
          - 3.7 -> 40
          - 3.8 -> 60
          - 3.95 -> 80
          - 4.2 -> 100
      - clamp:
          min_value: 0
          max_value: 100
    unit_of_measurement: "%"
    
binary_sensor:
  - platform: gpio
    pin: 
      number: 6 #P1.11
      inverted: true
      mode: INPUT
    name: "Ladevorgang abgeschlossen"
    filters:
      - delayed_on_off: 1s

  - platform: gpio
    pin: 
      number: 3 #P0.29
      inverted: true
      mode: INPUT
    name: "Ladevorgang aktiv"
    device_class: battery_charging
    filters:
      - delayed_on_off: 1s
```
...and built the code:

<p align="center">
<img width="1487" height="497" alt="image" src="https://github.com/user-attachments/assets/a8858051-d406-436f-b410-48ffc335ee9f" />
</p>

I was even able to flash it to the ESP32 via ESPHome builder and got a connection to Home Assistant via zigbee!

<p align="center">
<img width="360" height="302" alt="image" src="https://github.com/user-attachments/assets/799cfdc1-2bd2-464b-bc60-b9a90b599c73" />
</p>

Finally I was back where I started with the nrf52840. Something that actually shows up in my zigbee network.
However there were some - lets call it - inconveniences.

So first of all I do not want my battery level to show up as normal entity in Home Assistant. I want it to show up like this:

<p align="center">
<img width="368" height="305" alt="image" src="https://github.com/user-attachments/assets/73a4959d-c02c-44c2-a729-275edacc2001" />
</p>

And the second problem is, that I pull ~20mA from my 5V supply even without sensors attached. This causes also a bit of heat since 5V*20mA=100mW are significant losses for such a small microchip. At least high enough that I was able to check if the ESP32C6 was powered on by feeling the temperature.

The current draw also made me realize an oversight I have in my hardware design. I am powering all of my sensors from 3.3V at all times. Even if the microcontroller doesn't read any values. This will drain my battery quickly and sounds like a problem for my future self.

> Note from future self: Why are you doing this to me?

Anyways I had two options. Use the progress I made so far with ESP Home or find an alternative solution. I had only one day left before my custom PCBs arrived so it was time to finalize my code. So I made a list to help me with my decision. It is never good to rush into things.

|               | ESP Home YAML Route  | ESP-IDF Route  |
| ------------- | -------------------- | -------------- |
| Zigbee        | working              | unknown        |
| Temperature   | libs working         | unknown        |
| Weight        | libs working         | unknown        |
| Battery SoC   | working              | unknown        |
| Binary Output | working              | unknown        |
| Consumption   | Deepsleep available  | unknown        |

After I looked at this table for some time I just shrugged my shoulders, got another coffee and got to work.

#### ESP-IDF - The worst rated VS Code plugin I've ever used
> [!Important]
> This is not a recommendation. I never got it to work and it is not a beginner friendly route.

So yes after my short success with ESP Home it was of course only logical to go back into c-programming.
Sometimes I wonder if I am the polar opposite to current, since I am always searching for the path of highest resistance (small joke you learn as electrical engineer).

The steps are similar to the nRF52840. There is a VS Code extension I downloaded.
After I found the extension I got my first sign to turn back since the extension had not the best rating:

<p align="center">
<img width="382" height="82" alt="image" src="https://github.com/user-attachments/assets/2eb6e46d-5d8f-463b-9994-a42fb853b3ab" />
</p>

For comparison, this is the current rating of the PlatformIO IDE extension:

<p align="center">
<img width="409" height="91" alt="image" src="https://github.com/user-attachments/assets/b5fa1caf-7be2-40b3-8132-acbc2024e54e" />
</p>

I even asked copilot if would be better to use the Arduino IDE or PlatformIO IDE, but it told me to stop whining and use ESP-IDF.

I sighed, said "Here we go again" and hit install.
There is some documentation available that made the setup process easier. After following the installation steps and setting up the project I was ready to try my first blinky sketch (at this point I developed some ptsd from toggling LEDs).

Initially I installed the recent ESP-IDF v6.0.1 and again had to find out, that there is no zigbee library in that version. The same problem I had with the nRF52840. At that point the only thing I was able to do was uninstalling v6.0.1 and installing v5.5.4 while crying on the floor during that entire step.

> [!Tip]
> If you want to safe your time and tears, then just follow the official zigbee instruction from Espressif: https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/esp32c6/introduction.html

As described installing v5.5.4 is of course not the only step. I checked out the documentation and the libraries inside the installation folder only to find out, that the zigbee libraries were still missing.
I was really starting to get mad at this point. This was the second day of me sitting infront of my PC.

The last thing I did that night was cloning the esp-zigbee-sdk git repository and setting up one of the examples.
Finally I was able to flash something that at least included header files with "zigbee" in the name.
Again as always I was not able to find the device in Home Assistant. Even with the help of claude and copilot, the ESP32C6 is stuck in a neverending reboot loop:

<p align="center">
<img width="722" height="75" alt="image" src="https://github.com/user-attachments/assets/eca1d644-27b5-4a99-b878-747700be8f9b" />
</p>

So I gave up and went to bed. When I laid down I already heard the first birds sing. Maybe it was just my imagination going wild due to the lack of sleep, but I swear these birds are mocking me.

#### Why can I not connect to Home Assistant anymore?
It was the third day of tinkering with the ESP32C6. I got up from bed, turned on the PC and wanted to check if maybe Home Assistant was the root cause of the failed connection.
I was quite certain that it is not, since I was able to connect before when flashing via ESP Home, but since claude and copilot told me it is the root cause, I wanted to check anyway.

When I tried loading up the main page of Home Assistant I was greeted with a loading screen. What was going on?
I tried connecting with my tower PC (which I've been using so far), my laptop and my phone. Nothing..

<p align="center">
<img width="350" height="360" alt="image" src="https://github.com/user-attachments/assets/d98c2c16-3981-4e2c-aff3-60855ef263e4" />
</p>

So I got myself some coffee and on the way back to the PC I saw a yellow light on one of my wifi range extenders. Turns out the router which connects __all__ of my devices was "turned off" by an electrician doing some wiring in the apartment downstairs.

<p align="center">
<img width="500" height="375" alt="image" src="https://github.com/user-attachments/assets/6cfebaab-6dd6-45d2-907e-a9e958a54629" />
</p>

As shown in the picture it was out of the question to reconnect it. Even if it was reconnected, it would require the outlets to be powered on again, which were under construction.
Unfortunately for me, after some back and forth everyone came to the conclusion, that a delay in my project is less critical than a dead electrician. I really need to work on my negotiation skills.

I had no choice. I took my laptop, ESP32C6 and my phone so I could work remotely. The good thing was that I already pushed my latest progress to github. The bad thing: I still had to install all of the extensions to VS code again on my laptop. This also used up all of my phones data allowance.

After 1 hour of installation I was able to flash the ESP32C6 with my laptop.

#### Strg + C / Strg + V
All this waiting for installations to finish made me wonder about one thing. How does the actual C-code and the build environment look in ESP Home? What if I could just copy some initial sketch from there and adjust it to fulfill my needs?

I got my third coffee and had one goal. Find the project environment in my home assistant server and then copy it to my PC.
> [!Tip]
> The timing couldn't have been worse, because apparently it is not possible to ssh into a server, that is not powered on.

I don't want to go to much into detail but I do have a second Home Assistant server running at my parents house. I had no internet at home anyways so I've decided to change my physical development environment (which is my parents kitchen).

I copied the yaml into ESP Home of the second home assistant server and professionally analyzed the compiling process. Then I opened my terminal and ssh'ed into the server.

Here is a picture of what it looked like:

<p align="center">
<img width="320" height="320" alt="image" src="https://github.com/user-attachments/assets/bb35ce7e-9e69-4732-bfcc-d2d08f7d07b2" />
</p>

I did find my esphome files but as you can see there are three .yaml files and only two of them have additional folders. Any guesses which folder I wanted to see?

<p align="center">
<img width="2360" height="832" alt="image" src="https://github.com/user-attachments/assets/d7af0dee-7db9-4d52-a240-1f37635dfc1f" />
</p>

Yes, there was no folder with the files I was searching for. According to the build log in ESP Home there is supposed to be a build path here:
```
/data/build/beehivelive
```
And a python environment here:
```
/root/.platformio/penv/.espidf-5.5.4
```
They didn't exist either.. So I had to get another cup of coffee.

Overall I sat there in the kitchen until the sun went down and I learned the following facts the hard way:
1. Since some version (I forgot the exact one) ESP Home builds the code with containers
2. Home Assistant doesn't allow me to access these containers via SSH

What was my last option? Connect a monitor to my raspberry pi which runs my home assistant server. However, since the only available monitor was the TV my parents wanted to use, I was stuck. Again I had to find an alternative solution...

> [!Note]
> I really don't know what I was trying to achieve anyways. Getting the environment form the server didn't even mean I would be able to do something actually useful with it.

Believe it or not but I am actually extremely lazy. If I have the possibility to automate a 5 second task, I will gladly put 2 weeks of effort in some sort of McGyver automation to handle that task for me.
Still there is a limit of how much time I want to waste and I am starting to reach that limit.
I started up youtube and typed "ESP-IDF Zigbee" in the search bar to check if there was a tutorial to finally guide me properly to my goal but realized, that no one used the VS code ESP-IDF extension. Everyone was just using the Arduino IDE.

This meant I finally came back to my senses and started developing on the most reasonable approach: ESP32C6 + Arduino IDE

#### Arduino IDE might not be that bad after all
The statement "most reasonable approach" did not mean, that I would immediately write 1000 lines of code and just flash them without any other obstacles in my way.
On a positive note I was able to flash the Zigbee_Temperature_Sensor example immediately.
So far so good, it also showed up in Home Assistant and even reported a temperature reading of the internal sensor. (Time to reward myself with another cup of coffee)

However when I frankensteined the Zigbee_Temperature_Sensor and the Zigbee_Analog_Input_Output together, the device did not connect and I received loads of Zigbee Stack faults on the serial monitor.
Nothing I tried to change in the C-code seemed to work, except removing the _zbAnalogSensor.reportTemperature()_ function call.
There is a limited amount of faults I can handle per day so I went to bed and woke up the next day to the sound of an impact drill in the ceiling below me.

After a healthy amount of sleep I remembered one of the youtube tutorials. The creator said to always erase the flash before re-programming.
> [!Important]
> Tools -> Erase All Flash Before Sketch Upload -> Enabled

I selected the same settings and there it was. Finally an analog reading inside Home Assistant!

> [!Note]
> It is quite hot in the summer when you are living directly below the roof.

<p align="center">
<img width="364" height="257" alt="image" src="https://github.com/user-attachments/assets/2c6d8c83-4fc4-48bf-8cb9-18eba7c816fe" />
</p>

Of course it is not 183°C inside, this was just the raw ADC value of the battery voltage measurement (which was also not connected at this point). Initially I tried to get the battery SoC reading to show up again in the power configuration cluster. Apparently this is where this kind of information is supposed to show up for a zigbee device.

After digging through the Arduino libraries I even found an enumerator inside _esp_zigbee_zcl_common.h_ listing all the clusters, but unfortunately not all of them are ready to use "out of the box" like temperature or analog readings. I wasted another 3 hours tinkering and will solve this sometime in the future. Kind regards to my future self. 
> Note from future self: Again?

It is just another problem I can ignore until I will regret it. Similar to the backpain I developed 2 days ago.

For now the SoC is just shows up as percentage value. That should be good enough. It is calculated by a simple formula taking a voltage divider in my circuit and the analog resolution into account:
```
float voltage   = (adc_value / 1023.0f) * 3.3f * 1.9696066388226950354609929078014f;   // 1023: 10-bit adc / 3.3: max voltage / 1.96... voltage divider incl. inner gpio resistance
float soc = (voltage - 3.0f) * (100.0f / (4.11f - 3.0f));              // Li-Ion 3.0–4.11V → SoC 0–100%
```

<p align="center">
<img width="316" height="224" alt="image" src="https://github.com/user-attachments/assets/fe5fad6b-6b62-430c-b2da-39ada4de5522" />
</p>

As a next step I included the DS18B20 sensors I had still at home. I tried using the DallasTemperature library and found out that __again the libraries is not compatible with my ESP32C6__.

> [!Note]
> Using the newest version of a microcontroller can be quite challenging if you lack experience like me.

At least there is a 1-Wire library that is compatible and with [some code I stole from this forum](https://forum.seeedstudio.com/t/xiao-esp32c6-and-the-ds18b20/293778/10) I was able to finally transmit my first external temperature reading from the ESP32C6 to Home Assistant!

The downside is, it only works as long as there are not multiple sensors connected on the same 1-Wire bus. I threw a little party by myself in the room anyways.
Developing can be quite rewarding 0.5% of the time.

<p align="center">
  <img width="366" height="315" alt="image" src="https://github.com/user-attachments/assets/8ababb56-84fd-47c7-bb09-aea204044e6e" />
</p>

#### We need more sensors!
It was in the middle of the night and time to add two additional temperature sensors on the same 1-Wire Bus. I checked the [Official Arduino 1-Wire Documentation](https://docs.arduino.cc/learn/communication/one-wire/#addressing-a-1-wire-device) and added some code to read out the addresses of my sensors. I have overall 5 sensors and I already know the address of three of them:
```
0x8003213198B06C28
0xAB0321319EFABA28
0xEA0321319CC4C128
```

So it was also possible to verify the code in the same go since I should see at least one of these addresses. Absolutely awesome!
I used the serial monitor to show their addresses and connected my three sensors individually. Not to much trouble and I saw one familiar address:

<p align="center">
<img width="521" height="339" alt="image" src="https://github.com/user-attachments/assets/f0295c8b-de60-40ea-8427-e586a51e5394" />
</p>

> [!Important]
> What now follows shows quite clearly, why sleep and breaks are important (especially coffee breaks)!

Seeing the address shown from least significant byte to most significant in my serial monitor was triggering to me and so I decided to implement a quick fix in my code. Nothing special, just one altered line:
```
old: for(int i = 0; i < 8; i++)
new: for(int i = 8; i >= 0; i++)

correct: for(int i = 7; i >= 0; i--)
```

I don't want to exaggerate but there is only one programming mistake in human history that had similar consequences and that would be the self destruction of the [Ariane 5 Flight 501](https://en.wikipedia.org/wiki/Ariane_flight_V88).

The ESP32C6 did not like counting into integer overflow one bit and crashed on me over and over again. It is quite embarrassing how long it took me to find this bug, since I thought I caused timing issues due to my excessive use of _serial.print()_.
How did I come to this conclusion? Because the code didn't crash when I commented out my serial communication calls (which included the for loop).

<p align="center">
<img width="640" height="360" alt="image" src="https://github.com/user-attachments/assets/a1381e57-75f5-40b7-ab3f-380beecad223" />
</p>

> [!Tip]
> If it is already 3am and you are still debugging: Just go to bed.

#### Small steps to victory
It was wednesday during my summer vacation and the third day where the electrician downstairs made sure, that I will wake up after only 4 hours of sleep. You might ask yourself why I am not just getting to bed earlier and continue debugging in the morning? I would like to dodge that question and get back to more important topics.

I read through different forums and had conversations with Claude AI to learn a few things about 1-Wire. With that new knowledge I was able to connect all three temperature sensors at the same time and distinguish between them using their individual addresses. But there was once again a new challenge: Even after I implemented all of the three temperature readings as a different zigbee endpoint in my code and gave these individual endpoints they all showed up in Home Assistant just named "Temperature".

<p align="center">
<img width="320" height="208" alt="image" src="https://github.com/user-attachments/assets/ae595f62-1a83-4781-8c3f-09094f5970af" />
</p>

It was only possible to rename one of them by setting the model name of the first endpoint.
However, the entity names inside Home Assistant are still unique so I can still distinguish between them.

Also as a fun little side note. When I selected the analog cluster and selected an unitless count as an application _setAnalogInputApplication(ESP_ZB_ZCL_AI_COUNT_UNITLESS_OTHER)_ it was possible to change the name. So there seems to be some dependency on the application of the analog cluster. Thinking about it hurts my brain so I will just stop it.

Due to of all the debugging I did in that regard I was able to find out the solution to another earlier problem. Inside the _ZigbeeEP.h_ header file I found the following functions:
````
// Set Power source and battery percentage for battery powered devices
  bool setPowerSource(zb_power_source_t power_source, uint8_t percentage = 0xff, uint8_t voltage = 0xff);  // voltage in 100mV
  bool setBatteryPercentage(uint8_t percentage);                                                           // 0-100 %
  bool setBatteryVoltage(uint8_t voltage);                                                                 // voltage in 100mV (example value 35 for 3.5V)
  bool reportBatteryPercentage();                                                                          // battery voltage is not reportable attribute
````
These functions allow an integration of battery information into the power configuration cluster defined in the [Zigbee Cluster Library Specification](https://zigbeealliance.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf).

Or in another words that even I was able to understand: Battery full or not go from ESP32 to here:

<p align="center">
<img width="369" height="305" alt="image" src="https://github.com/user-attachments/assets/75e42a18-5615-400f-b18d-4dbf21274445" />
</p>

The only success in human history comparable to this milestone was the [Apollo 11 Mission](https://en.wikipedia.org/wiki/Apollo_11).
Apollo 11 cost around 355 Million US-Dollar taken inflation into account. I got my battery level reading into Home Assistant with just a fraction of the effort or cost. However, I believe I might have used the same amount of caffeine.

Regarding the HX711 weight sensor it was quite simple to get it into Home Assistant, since I was able to reuse some code of other sensors to define a new zigbee end device. After that it was only required to add 2-3 lines of code. Everything else is handled in the background by the [HX711 library](https://github.com/bogde/HX711).

There was one last problem left: The only suited zigbee cluster I had available was the analog input cluster. This cluster however does not feature any weight application. The available options are stored in the _esp_zigbee_zcl_analog_input.h_ header file.
````
/** @brief Values for Analog Input cluster applications type*/
typedef enum {
    ESP_ZB_ZCL_AI_APP_TYPE_TEMPERATURE,        /*!< Temperature */
    ESP_ZB_ZCL_AI_APP_TYPE_HUMIDITY,           /*!< Humidity */
    ESP_ZB_ZCL_AI_APP_TYPE_PRESSURE,           /*!< Pressure */
    ESP_ZB_ZCL_AI_APP_TYPE_FLOW,               /*!< Flow */
    ESP_ZB_ZCL_AI_APP_TYPE_PERCENTAGE,         /*!< Percentage */
    ESP_ZB_ZCL_AI_APP_TYPE_PPM,                /*!< Ppm */
    ESP_ZB_ZCL_AI_APP_TYPE_RPM,                /*!< Rpm */
    ESP_ZB_ZCL_AI_APP_TYPE_CURRENT_IN_AMPS,    /*!< Current In AMPS */
    ESP_ZB_ZCL_AI_APP_TYPE_FREQUENCY,          /*!< Frequency */
    ESP_ZB_ZCL_AI_APP_TYPE_POWER_IN_WATTS,     /*!< Power In Watts */
    ESP_ZB_ZCL_AI_APP_TYPE_POWER_IN_KILOWATTS, /*!< Power In Kilowatts */
    ESP_ZB_ZCL_AI_APP_TYPE_ENERGY,             /*!< Energy */
    ESP_ZB_ZCL_AI_APP_TYPE_COUNT_UNITLESS,     /*!< Count Unitless */
    ESP_ZB_ZCL_AI_APP_TYPE_ENTHALPY,           /*!< Enthalpy */
    ESP_ZB_ZCL_AI_APP_TYPE_TIME,               /*!< Time */
    /* Types 0x0f to 0xfe are reserved */
    ESP_ZB_ZCL_AI_APP_TYPE_OTHER = 0xff, /*!< Other */
} esp_zb_zcl_ai_application_types_t;
````

> [!Note]
> I do not know what the reasoning behind this is. Maybe there is simply no usecase for someone to transmit his weight from a scale in the bathroom directly to his smart home system through a low power smart device mesh. Eventhough the longer I think about it, the more interesting it seems.

If there was no possibility to display weight in kg I wanted to at least remove the unit completely. I tried the following applications:

| Application | Unit shown in HA |
| ----------- | ---------------- |
| ESP_ZB_ZCL_AI_APP_TYPE_OTHER | °C |
| ESP_ZB_ZCL_AI_APP_TYPE_COUNT_UNITLESS | counts |

As shown in the table the only two applications that seemed useful turned out to be useless. But then something happened. There was the perfect combination of low blood sugar, amount of caffeine and lack of sleep in my system and then I saw this line of code in the header file:
````
#define ESP_ZB_ZCL_AI_SET_APP_TYPE_WITH_ID(_type, _id) (((_type & 0xff) << 16) | (_id & 0xffff))
````
<p align="center">
<img width="400" height="400" alt="image" src="https://github.com/user-attachments/assets/9c5f8464-3881-4bb8-ac52-b1991369e27a" />
</p>

So I just entered 0xffffff instead of an application (this is different from _ESP_ZB_ZCL_AI_APP_TYPE_OTHER_ which corresponds to 0xff0000) and it worked perfectly. All temperatures and the raw weight reading from the HX711 were transmitted to Home Assistant via zigbee.

It was time for another party!

#### Binary information of solar charger
It was friday, so the last day for me to hear the sound of an impact drill.

The CN3791 solar charging IC I built into my PCB features two open-drain outputs. One of them is active when solar charging is active and the other one when the battery is fully charged. In my case they are connected to an individual pull-up resistor and individual pin of the ESP32C6. By reading the binary status of those pins it is possible to know what the solar charging IC is doing.

Adding binary informations of the solar charger and transmit them via Zigbee was quite easy to implement, since binary sensors are common in Zigbee devices (Door/Window switches, HVAC on/off, ...).  I took code from the Zigbee_Binary_Input_Output sketch and adjusted it to my needs and within 5min I saw the solar information in Home Assistant.

#### Live calibration of deep sleep and weight sensor
Since the load cell is already below one of the bee hives it makes sense to implement the possibility to send the correct gain and offset of the scale via zigbee to the ESP32C6. The code to implement an entity to send data from Home Assistant to the end device is quite similar to the previous _analog input_ definitions and just called _analog output_. It is also avaliable in the zigbee examples inside the Arduino IDE. Overall I have three analog outputs and therefore calibratable values.
1. Deep Sleep Duration
2. HX711 Gain
3. HX711 Offset

As the name implies it is also possible to adjust the deep sleep duration of the ESP32C6 to safe some battery life. Once the ESP32C6 goes into deep sleep the power consumption is greatly reduced. Upon wakeup the code starts from the beginning and reruns the complete setup routine.

That also introduced a new issue: Once the ESP32 wakes up from deep sleep, it has forgotten all of it's previous variables. The previously set weight sensor gain, offset and the deep sleep duration are reset. Fortunately the Zigbee communication restarts without going through the pairing process in the Zigbee Home Assistant integration again. However any Analog Output value (sent from Home Assistant to the end device) is received as "0" after deep sleep.

> [!Note]
> There might be some sophisticated solution to this problem I could find after spending a few more hours and drinking more coffee, but if you got this far in my documenation you might have already realized, that I am not the definition of sophisticated.

To solve this issue I added the [Preferences library](https://github.com/vshymanskyy/Preferences) to my code and stored all relevant information inside the microcontrollers NVS. This way I can access this information even after losing the power supply to the ESP32C6.
If the end device reads a value 0 from Home Assistant it will ignore this value and use the last value from the NVS.

From this point on I was finally able to integrate the ESP32C6 into my custom PCB, which means from this point on we are leaving the software development stage and go into the system integration (e.g. combining hardware and software).

## System Integration
### Initial setup and weight sensor calibration
I connected the end device to Home Assistant and connected everything (1x solar panel, 3x temperature sensors and 1x load cell). Initially I did not activate deep sleep to make it easier to calibrate the weight measurement first.

For the calibration I simply reused the offset I already had in the previous project. It is already stated as weight offset in kg. For the gain it was required to recalibrate based on a known weight. I initially used the gain of the previous project and compared to measurements. The first measurement was only the weight of the bee hive. The second measurment was the weight of the bee hive + a 10kg weight I placed on top of the bee hive:

<p align="center">
<img width="472" height="356" alt="image" src="https://github.com/user-attachments/assets/1428d7f4-9e8f-49bb-b70b-f4d7020a51f1" />
</p>

Based on the two measurments it is possible to recalculate the required gain and sent it to the ESP32 via Home Assistant.

### Connectivity issues after deep sleep
After everything was set up and working I sent the ESP32C6 to deep sleep with a duration of 5 minutes.
I waited in front of my screen waiting for the next measurement... 5 minutes passed.. 10 minutes passed.. I got myself some coffee.. 30 minutes passed.. I traveled to Japan to eat some wagyu beef in my favorite restaurant in Osaka and got back home.. no new measurement.

I reflashed and reconnected the ESP32 to Home Assistant multiple times and had always the same issue. As long as the device was not in sleep I was barely able to keep a stable connection. Once I went to sleep and the ESP32 had to rejoin the zigbee network, it never got a stable connection.

Optimizing the code did not help to much. I increased the transmission power of the transceiver using the following line of code:
```
esp_zb_set_tx_power(IEEE802154_TXPOWER_VALUE_MAX);
```
I even tried cycling through different transmission powers in case the communication failed without any success. Also masking the zigbee network channel to prevent searching through the wrong channels showed no improvement.

> [!Tip]
> Masking the zigbee channels only makes sense for the initial pairing. Once the device was paired once the network information is stored in the zigbee stack.

Nothing I tried seemed to work and I came to one conclusion: You cannot compensate a wrong hardware design decission with software.

### Is my antenna even activated?!
I started to do some research on how to increase the range of my device and the initial plan was to change my setup to LoRA communication which can achieve multiple kilometers of range. After all with my design it is possible to use any 14-pin seeed studio microcontroller and breakout board.
A great solution would have been the [XIAO nRF52840 & Wio-SX1262 Kit for Meshtastic](https://www.seeedstudio.com/XIAO-nRF52840-Wio-SX1262-Kit-for-Meshtastic-p-6400.html).

But after checking the [Seeed Studio documentation for the ESP32C6](https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/#hardware-overview) I stumbled on my problem: I never activated my external antenna...

So I added the following code to activate it and the now connection works and is stable:
````
void initAntenna() {
  // Seeed Studio ESP32C6 features an internal and external antenna
  // Activate antenna selection
  pinMode(RF_SWITCH_POWER, OUTPUT);
  digitalWrite(RF_SWITCH_POWER, LOW);

  delay(100);

  // Select external antenna
  pinMode(RF_SWITCH_PORT_SELECT, OUTPUT);
  digitalWrite(RF_SWITCH_PORT_SELECT, HIGH);
}
````

### Charging feedback issues
After my vacation was over and I was back in the office the bee hive sensor operated for one week without other adjustments. I checked in every day to monitor the weight and temperature of my bees and the charging behavior of the solar charging circuit.
I noticed that the charging SOC stopped at ~90% with unstable readings:

<p align="center">
<img width="1441" height="557" alt="image" src="https://github.com/user-attachments/assets/3c2ba710-4d54-4d2c-9464-3101d49c725f" />
</p>

Additionally, during the complete week there was no feedback from the charging IC CN3791.
In case the Li-Ion cells are fully charged the "Done" output of the CN3791 should be pulled low which is also monitored by the ESP32C6.
At least during sun hours either the "Done" output or the "Charge" output should be pulled low.

Since the ESP32C6 was still operating I assumed, that the ESP32C6 and the 3.3V-LDO between Li-Ion cells had to be fine. Also the Li-Ion cells themselves obviously provided power to the ESP32 and the sensors.
I had a few assumtions I wanted to test on the weekend:
1. Is the PV-Panel broken?
2. Is there a short in the wiring between PV-Panels and my circuit? (Maybe the bees now eat wiring for some reason?)
3. Is the MPPT voltage on my circuitry still set correctly?
4. Are there broken components in the circuit?

Point 1 and 2 were easy to check. I was able to see the voltage on the input of my circuit.
As a next step I took the bee hive sensor to my bench, opened it up and realized, that the batteries were quite warm.
Touching them by hand was possible but the temperature was so high that I initially assumed a short or overvoltage at the cells.
I took them out and checked the temperature with thermal imaging. Unfortunately, it took me about 15 minutes to get the camera, so the cells already cooled back down:

<p align="center">
<img width="350" height="450" alt="image" src="https://github.com/user-attachments/assets/56b2c7ba-87a2-4100-9f04-72bd623ae911" />
</p>

> The question is: If the cells cooled back down to ~29.5°C after standing on my workbench for 15 minutes, how hot where they when I first opened the enclosure?
> Let's procrastinate shortly for this small calculation exercise!
>
> To answer this question we need to find out two things:
> 1. How much heat can be stored inside a typical Li-Ion 18650 cell or in other words: What is its thermal capacity?
>    - I am going to assume a thermal capacity of 1000 J/(kg*K) based on [this source](http://batterydesign.net/specific-heat-capacity-of-lithium-ion-cells/?srsltid=AfmBOopFGdwng84xYbtfqtYw4F1H269hViXRv6glTufAXOPyDeTv4uzo).
> 2. How fast does the Li-Ion cell radiate its heat to the environment? What is its heat transfer coefficient?
>    - I am going to asume 14 W/(m²*K) as heat convection coefficient based on [this study](https://www.sciencedirect.com/science/article/abs/pii/S1359431122014892).
>    - As a simplification I am only taking heat convection into account
>   
> If we have a temperature difference between ambient air and the cell we can calculate the power transfered between them using the heat convection coefficient. That transferred power over time will cause a change in temperature of the Li-Ion cell due to its thermal capacity and the stored energy as heat.
> Using an excel document I created a function of the temperature over time by recalculating the power transfer and stored heat in discrete time steps.
> The time axis nof the graph describes the cooldown duration. At 15min the estimated temperature 15min before the actual measurment is shown:
>
> <p align="center">
> <img width="2371" height="837" alt="image" src="https://github.com/user-attachments/assets/42f9b081-21cd-48f4-a1f4-cf44c6585325" />
> </p>
> 
> So to answer the question. When these assumptions are correct, the 18650 Li-Ion cells were above 50°C!

There was no short circuit in the PCBA and also the cell voltage was sitting stable at 4.11V. Therefore the most likely explaination is the direct sunlight hitting my bee hive sensor and heating up the cells through the see-through cover.
From now on the assembly will not be placed in direct sunlight anymore.

To get back to the actual issue: 
I checked the solar charging circuit by measuring the diodes D1 and D2. I also checked if the output voltage and input voltage was stable. After I did not find an issue with these components I moved on to the Mosfet M1 and the CN3791 itself.

<p align="center">
<img width="429" height="299" alt="image" src="https://github.com/user-attachments/assets/e8b7ca57-f210-4302-a1a8-1426ce700818" />
</p>

To check if the CN3791 properly drives the Mosfet, I connected my power supply to the PV-Input and set it to 20V. Afterwards I measured the voltage on the MPPT-Pin of the CN3791 which was 1.42V. According to the datasheet of the CN3791 the pins voltage is regulated to 1.205V. This means if the voltage on this pin 6 is higher than 1.205V, the output is active and the CN3791 controls the gate of the P-Channel MOSFET "M1" through the pin 10. That means that the MPPT voltage is still set correctly and there is no damage to the voltage divider.

> [!Note]
> The P-Channel MOSFET gives me a few headaches eversince the charging stopped working. When I went through every component to find a possible root cause for charging problems I realized, that this mosfet is only rated for a drain-to-source voltage of 20V.
>
> It is a design mistake I did due to misplaced trust into component selection filters. In the component selection I filtered for 40V rated mosfets and never double-checked with the actual datasheet. So now I have this 20V mosfet in my design operating slightly above its maximum rating. If you ever meet me in person then please don't bring it up. I have still nightmares because of this.
>
> Of course it is already fixed in my schematic in case I ever have to order new PCBAs.

Next I used my most advanced and high end equipment on hand to check the drive output pin 10 of the CN3791 IC. As shown on the oscilloscope the drive output also works just fine:

<p align="center">
<img width="635" height="470" alt="image" src="https://github.com/user-attachments/assets/81621ff9-e3fa-49f3-8e38-d016c1e881de" />
</p>

During that measurement the Li-Ion cells were also charged without any issues, so everything is suddenly fine?
Well the problem was actually, that the cell voltage of 4.11V was already picked up by the CN3791 as "fully charged". It pulled the "Done" pin 4 low and stopped charging.
I was supposed to see the the status of this pin as described in the software section, however it seems like I found a software bug.

As long as the ESP32C6 is not using deep sleep, the status of the "Done" and "Charge" pin on the CN3791 are transmitted via zigbee. Once deep sleep is active, these binary states are frozen.
I was also able to verify this using the charge pin. Since it is currently half past 9 and my ESP32C6 still claims to be charging.
So either my bee hive sensor is lying or I made a brakethrough in solar power by using light of other stars than the sun to charge?

### Cell voltage measurement noise
To fix the unstable SOC readings I went back into my design and added a capacitor to the analog input of the ESP32C6 to filter the signal.
Since I am not going to order a new PCBA for every single small mistake I fixed this issue on my current design by adding a THD capacitor to the back of my ESP32.

<p align="center">
<img width="255" height="231" alt="image" src="https://github.com/user-attachments/assets/803a8acb-24aa-4f2d-927b-dea415485d76" />
</p>

From a decoupling/filtering point of view this was far from the best solution, since the long legs on the capacitor add parasitic resistance and inductance. But at least for the problematic low frequency noise this improved the measurment significantly.
Red shows the measured SOC without capacitor. Green shows the measurement after adding the capacitor for signal filtering:

<p align="center">
<img width="2289" height="447" alt="image" src="https://github.com/user-attachments/assets/5b706032-62eb-4cd5-a21b-29c8bd924d25" />
</p>
