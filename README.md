# BeeHiveLive
This respository describes a bee hive monitor that is integrated to Home Assistant as a zigbee end device. The bee hive monitor is a custom made PCBA that includes an I2C interface for temperature/humidity measurement, load cell for weight measurement and solar charging circuitry for Lithium-Ion batteries.
> [!Note]
> It is almost 2am at night and my brain is not properly working anymore, so I would like to apologize for this documentation.
> Also there might be some sarkasm inside this document since the whole process makes me question my sanity.
>
> Let's see where this will get us...

## Software Development
> I have no idea how to programm so I basically just steal code and torture every AI known to man to get my code running.
> It is the software equivalent of using Duck Tape in order to mount a jet engine to a Boeing 787-9 Dreamliner.

### Step 1: Being a failure at using the nrf52840
I planned to use a seeed studio brakeout board based microcontroller that features Zigbee connectivity.
That enables me to connect the device to my already established Zigbee mesh.
After a short search I stumbled upon two different options:
1. Seeed Studio XIAO ESP32C6: https://wiki.seeedstudio.com/xiao_esp32c6_getting_started/
2. Seeed Studio XIAO NRF52840: https://wiki.seeedstudio.com/XIAO_BLE/

After some googling "ESP32C6 vs NRF52840 current consumption" it turns out, that the NRF52840 is supposed to have a really low power comsumption, making it absolutely perfect for my application.
Even something about compatibility with ESP Home and the Arduino IDE is written in the documentation, so I knew that I will have an easy time programming it (Foreshadowing can be quite obvious).

#### The limitations of ESP Home and yaml
It is possible to generate code in ESP home and it actually has such a low power consumption, that I was not even able to pick it up with my USB power meter.
However there is one problem. I want to be compatible with One-Wire devices, I2C and an HX711 weight sensor.
At first I tried implementing one of my dallas temperature sensors using the One-Wire protocoll.
I fired up the code generation in ESP Home and after exactly 93.82 seconds my professional "ESPHome-YAML-Developer" career came to an end:

<img width="2010" height="98" alt="image" src="https://github.com/user-attachments/assets/df24ccdc-0d2e-4281-a9fc-826220a76eed" />

I am not going to pretend to know what happened here, but my friend who works as a copilot for microsoft knows whats up.
There is currently no library for the usage of one wire devices with the NRF52840 chip in ESP Home.
At least it looks like smarter people than me are also aware of that problem: https://github.com/esphome/esphome/issues/16163

So I just scraped the ESP Home approach for now and used something that lets me use my full c-programming potential. After all, ESP Home might just be dragging me down (might include sarcasm).

#### The Savior? - An avaliable library for NRF52840 in the Arduino IDE
The last time I used arduino was back in universtity. There was only a white background for the IDE which caused me to delete it.
A real developer needs a dark mode and lines of text flying through a terminal after all.

However since the Arduino IDE is aimed towards beginners like me I wanted to give it a try.
Indeed there is a library avaliable to get started with my specific NRF52840 board. 
So I added the board URL to my preferences and tried out example sketches.

<img width="994" height="663" alt="image" src="https://github.com/user-attachments/assets/d85e8646-102b-4b85-9245-276259d98ea5" />

> [!Important]
> I am not going to paste the URL here, since this approach will not work anyways and I was running straight against a wall here. Unforfunately my past self didn't know this yet and spent 4 hours of debugging and looking through forums before finally giving up.

It turns out that programming zigbee to the nrf52840 is not possible in the Arduino IDE. Again my copilot friend from microsoft told me this.
Appearently since some version of the nRF Connect SDK, zigbee is not supported anymore for the nrf52840 chip.
I did not even try to do the same thing in PlatformIO for Visual Studio Code, since there I was greeted with two issues:
1. For connectivity only bluetooth shows up
2. I cannot even find my Seeed Studio XIAO nrf52840 board
3. 
<img width="1349" height="102" alt="image" src="https://github.com/user-attachments/assets/1d6770c4-67f7-4ed2-9a86-60dcbefad9e8" />

#### Going full professional since the Arduino IDE is for beginners
I guess after 14+ hours without sleep infront of three monitors with one of them only playing music videos, I can call myself professional developer. I somehow feel the urge to go to a TED talk and tell people about my journey.

Back to topic: So the recommended way by the people who built the nrf52840 is the usage of Visual Studio Code + nRF Connect SDK.
Awesome stuff and let me tell you: Now the real pain begins.

In fact I had to use Ibuprophen 400 just to keep myself motivated.
> [!Warning]
> This is just a joke. Don't do drugs, it will ruin your code. (Except caffein. Do lots of caffein. Nobody trusts a programmer that doesn't have a coffee IV in their arm)

I played around with the environment and found out the following facts:
1. Zigbee is not avaliabe in every SDK version -> v2.9.2 seems to have the required libraries
2. It is extremely important where you create your project folder

When buliding code the automatically generated folder structure grows a lot. So large in fact that the code generation will fail due  to the amount of symbols in the filepath (now we know the maximum is 250 symbols).

3. Zigbee example projects do not allow the usage of the Seeed Studio board with its bootloader

The Seeed Studio version acts like a usb drive. When you double press reset, it shows up in the explorer and allows you to drag .uf2 files to it. These are not generated by the examples by default.
> [!Caution]
> This next step fried my bootloader and I do not have the required hardware to flash a new one.

Fortunately (I thought to myself) it is possible to bulid them by adding this line in the prj.conf file:
```
CONFIG_BUILD_OUTPUT_UF2=y
```
Finally after hours of debugging I got my .uf2 file. You cannot imagine how impressed I was by myself at this point.
I pulled the .uf2 into my nrf52840 and checked wheather I could find it in Home Assitant as zigbee device.
After staring at the screen for about 10 minutes I tried to generate a simple blinky sketch in the same environment.
The LED stayed dark and I fell into a pit of rage.

After calming myself down by watching videos of cats taking catnip, I did some investigation.
My guess to what happened is, that I never defined how this uf2 file is supposed to be structured.
For comparison, this is how a .uf2 file is supposed to look like:

<img width="551" height="27" alt="image" src="https://github.com/user-attachments/assets/5c7e2f65-90aa-47ec-8651-97c2d2a9a1bd" />

And this is what my sleep deprived brain came up with:

<img width="530" height="26" alt="image" src="https://github.com/user-attachments/assets/d36a2f4b-3699-43ad-ba6a-de24c32204c3" />

I wonder what is usually stored between 0x00000 and 0x27000...

In the next few hours I tried to get the blinky sketch running on the nrf52840 using other build environments. First a proper Visual Studio Code + nRF Connect SDK environment. Afterwards just the Arduino IDE and ESPHome.
It took me quite some time to realize my problem, since the board still shows up as a USB device when I double-click the reset button. It ignores every .uf2 file I try flashing on it. I even wondered at some point if my built-in LED is just broken and so I hooked up a logic analyzer to the D0 Pin to check if it will toggle:

<img width="2111" height="189" alt="image" src="https://github.com/user-attachments/assets/5737b3a4-8dfd-4967-be1f-efa5b4fd9ab9" />

Of course it doesn't. I had just bricked my microcontroller.
Impressive how far you can make it with a masters degree in electrical engineering.

Let's try to destroy the ESP32C6 next!

### Step 2: ESP32C6 is easy to programm if only I knew how to do it
#### ESP Home works best for ESP devices for some reason
I've decided to go back to ESP Home and yaml files to generate my code, since bricking the nrf52840 heavly bruised my ego.
Hopefully the ESP32 is much easier to programm.

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

<img width="1487" height="497" alt="image" src="https://github.com/user-attachments/assets/a8858051-d406-436f-b410-48ffc335ee9f" />


I was even able to flash it to the ESP32 via ESPHome builder and got a connection to Home Assistant via zigbee!

<img width="360" height="302" alt="image" src="https://github.com/user-attachments/assets/799cfdc1-2bd2-464b-bc60-b9a90b599c73" />

Finally I was back where I started with the nrf52840. Something that actually shows up in my zigbee network.
However there were some - lets call it - inconvieniences.

So first of all I do not want my battery level to show up as normal entity in Home Assistant. I want it to show up like this:

<img width="368" height="305" alt="image" src="https://github.com/user-attachments/assets/73a4959d-c02c-44c2-a729-275edacc2001" />

And the second problem is, that I pull ~20mA from my 5V supply even without sensors attached. This causes also a lot of heat since 5V*20mA=100mW is a lot for such a small device.

The current draw also made me realize an oversight I have in my hardare design. I am powering all of my sensors from 3.3V at all times. Even if the microcontroller doesn't read any values. This will drain my battery quickly and sounds like a problem for my future self.

Anyways I had two options. Use the progress I made so far with ESP Home or find an alternative solution. I had only one day left before my custom PCBs arrived so it was time to finalize my code. So I made a list to help me with my decission. It is never good to rush into things.

|               | ESP Home YAML Route  | ESP-IDF Route  |
| ------------- | -------------------- | -------------- |
| Zigbee        | working              | unknown        |
| Temperature   | libs working         | unknown        |
| Weight        | libs working         | unknown        |
| Battery SoC   | working              | unknown        |
| Binary Output | working              | unknown        |
| Consumption   | Deepsleep avaliable  | unknown        |

After I looked at this table for some time I just shrugged my shoulders, got a bottle of spezi and got to work.

#### ESP-IDF - The worst rated VS Code plugin I've ever used
So yes after my short success with ESP Home it was of course only logical to go back into c-programming.
Sometimes I wonder if I am the polar opposite to current, since I am always searching for the path of highest resistance (small joke you learn as electrical engineer).

The steps are similar to the nRF52840. There is a VS Code extension I downloaded.
After I found for the extension I got my first warning to turn back since the extension had not the best rating:

<img width="763" height="164" alt="image" src="https://github.com/user-attachments/assets/2eb6e46d-5d8f-463b-9994-a42fb853b3ab" />

For comparison, this is the current rating of the PlatformIO IDE extension:

<img width="818" height="181" alt="image" src="https://github.com/user-attachments/assets/b5fa1caf-7be2-40b3-8132-acbc2024e54e" />

I even asked copilot if would be better to use the Arduino IDE or PlatformIO IDE, but it told me to stop whining and use ESP-IDF.

I sighed, said "Here we go again" and hit install.
There is some documentation avaliable that made the setup process easier. After folowing the installation steps and setting up the project I was ready to try my first blinky sketch (at this point I developed some ptsd from toggling LEDs).

Initially I installed the recent ESP-IDF v6.0.1 and again had to find out, that there is no zigbee library in that version. The same problem I had with the nRF52840. At that point the only thing I was able to do was uninstalling v6.0.1 and installing v5.5.4 while crying on the floor during that entire step.

> [!Tip]
> If you want to safe your time and tears, then just follow the official zigbee instruction from Espressif: https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/esp32c6/introduction.html

As described installing v5.5.4 is of course not the only step. I checked out the documentation and the libraries inside the installation folder only to find out, that the zigbee libraries were still missing.
I was really starting to get mad at this point. This was the second day of me sitting infront of my PC.

The last thing I did that night was cloning the esp-zigbee-sdk git repository and setting up one of the examples.
Finally I was able to flash something that at least included header files with "zigbee" in the name.
Again as always I was not able to find the device in Home Assistant. Even with the help of claude and copilot, the ESP32C6 is stuck in a neverending reboot loop:

<img width="722" height="75" alt="image" src="https://github.com/user-attachments/assets/eca1d644-27b5-4a99-b878-747700be8f9b" />

So I gave up and went to bed. When I laid down I already heard the first birds sing. Maybe it was just my imagination going wild due to the lack of sleep, but I swear these birds are mocking me.

#### Why can I not connect to Home Assitant anymore?
It was the third day of tinkering with the ESP32C6. I got up from bed, turned on the PC and wanted to check if actually Home Assitant was the root cause of the failed connection.
I was quite sure that it is not, since I was able to connect before when flashing via ESP Home, but since claude and copilot told me this is the root cause, I wanted to check anyway.

When I tried loading up the main page of home assitant I was greeted with a loading screen. What was going on?
I tried connecting with my tower PC (which I've been using so far), my laptop and my phone. Nothing..

<img width="899" height="935" alt="image" src="https://github.com/user-attachments/assets/d98c2c16-3981-4e2c-aff3-60855ef263e4" />


So I got myself some coffee and on the way back to the PC I saw a yellow light on one of my wifi range extenders. Turns out the fuse to the router which connects all of my devices was turned off by an electrician doing some wiring in the apartment downstairs. Unfortunately for me, after some back and forth everyone came to the conclusion, that a delay in my project is less critical than a dead electrician. I really need to work on my negotiation skills. 

I had no choice. I took my laptop, ESP32C6 and my phone so I could work remotely. The good thing was that I already pushed my latest progress to github. The bad thing: I still had to install all of the extensions to VS code again on my laptop. This also used up all of my data allowance.

After 1 hour of installation I was able to flash the ESP32C6 with my laptop.

#### Strg + C / Strg + V
All this waiting for installations to finish made me wonder about one thing. How does the actual C-code and the build environment look in ESP Home? What if I could just copy some initial sketch from there and adjust it to fullfil my needs?

I got my third coffee and had one goal. Find the project environment in my home assistant server and then copy it to my PC.
> [!Important]
> The timing couldn't have been worse, because appearently it is not possible to ssh into a server, that is not powered on.

I don't want to go to much into detail but I do have a second home assitant server runnning at my parents house. I had no internet at home anyways so I've decided to change my physical development environment (which is my parents kitchen).

I copied the yaml into ESP Home of the second home assistant server and professionally analyzed the compiling process. Then I opened my terminal and ssh'ed into the server.

Here is a picture of what it looked like:

<img width="320" height="320" alt="image" src="https://github.com/user-attachments/assets/bb35ce7e-9e69-4732-bfcc-d2d08f7d07b2" />


I did find my esphome files but as you can see there are three .yaml files and only two of them have additional folders. Any guesses which folder I wanted to see?

<img width="2360" height="832" alt="image" src="https://github.com/user-attachments/assets/d7af0dee-7db9-4d52-a240-1f37635dfc1f" />

Yes, there was no folder with the files I was seraching for. According to the build log in ESP Home there is supposed to be a build path here:
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

What was my last option? Connect a monitor to my raspberry pi which runs my home assistant server. My parents were watching TV but now they had to watch the file system of home assistant.

> [!Note]
> I really don't know what I was tying to achieve. Getting the environment form the server didn't even mean I would be able to do something actually usefull with it.

## Hardware Development
Yes I developed custom PCBs for the project.
## Testing
Testing of the 
