# Flysky PL18 & PL18EV Hardware Mod for Complete EdgeTX Support

With some hardware modifications, it is possible to run a full featured EdgeTX firmware on Flysky PL18 and Paladin EV radios. Natively, these radios have the following limitations with EdgeTX:

1. PL18/PL18EV have limited internal memory that is not big enough to carry a full SD card content. PL18/PL18EV do not have an option to expand the internal memory with a microSD card. Internally, these radios carry a 8 MByte sized NOR flash chip, which is sufficient for EdgeTX configuration files, but not for a full EdgeTX experience. For example, the std. English sound pack w/o explicit script sounds, needs already ca. 26 MBytes.
2. PL18/PL18EV cannot output arbitrary sounds. Instead they use a dedicated sound chip with factory pre-programmed sounds. And it is also not easily possible to reprogram this chip, e.g. using a software only solution.
3. PL18/PL18EV cannot keep by default the date and the time. PL18 and PL18EV internal printed circuit boards are prepared for real-time-clock functionality, but the required battery is not populated by default. For this reason, RTC functionality will not work properly with EdgeTX out of the box.

Below you can find step-by-step tips of how to modify your PL18/PL18EV to overcome the above mentioned problems. Be aware that warranty will be most certainly lost when following the modification steps listed here.

Craig at WTF RC Cars has created a nice video, visually explaining the hardware modification steps (click on the picture below to open the video):

[![Video of the steps](https://i.ytimg.com/vi/r_UN9fZ-rDw/hqdefault.jpg)](https://www.youtube.com/watch?v=r_UN9fZ-rDw "Edge TX PL18 hardware mods RTC battery. Sound and memory upgrade.")

### Step 1: Install RTC battery and upgrade the flash chip

Open up your radio and take out the MCU main PCB that is located right under the LCD display. The MCU PCB is shown here:

![PL18 MCU PCB](https://github.com/EdgeTX/edgetx/assets/18414861/61dfcc52-7f3c-4c79-8f01-d59529d38598)

The RED CIRCLE denotes the location of the RTC battery. This can be populated with MS-621 rechargeable battery. Please note that after installation, it needs some time for it to charge up, before RTC functionality starts to work properly.

The BLUE SQUARE denotes the NOR flash chip. The factory chip populated is of type W25Q64JV (8MB in size) and is not big enough to fit all required files of the EdgeTX SD card content. You can replace it with MX25L25645G that has the same form factor and pinout, but is 32MB in size. Thus, desolder the W25Q64JV and replace it with MX25L25645G. Be sure to orient the IC the same way when resoldering (match pin 1 location).

(Updated 2023-10-07) We discovered that the chips that uses WSON8 packaging can fit for replacement as well. However this packaging is better soldered using a hot blower instead of a soldering iron and can be an option to those who are skillful enough. And the latest branch supports W25Q512JV (64MB) and W25Q01JV (128MB) NOR flash chips as well.

### Step 2: Mod for DAC sound output

This modification reroutes the sound generation directly to the main STM32F4 microcontroller, bypassing the original factory pre-programmed audio chip. This way arbitrary sounds can be output via EdgeTX firmware. The modification is carried out on the PCB in the lower part of the radio. As a first step, disconnect the output of the sound chip to the audio amplifier by de-soldering the capacitor marked with a RED CIRCLE in the following image:

![Capacitor from audio chip to audio amplifier](https://github.com/EdgeTX/edgetx/assets/18414861/36de6855-ff27-41fc-ba94-75742f4fbf2a)

As a second step, wire up the DAC output to the sound amplifier input via a capacitor as shown below:

![DAC connection to audio amplifier](https://github.com/EdgeTX/edgetx/assets/18414861/5defa6ad-ba4d-4da8-9fe7-61e7c69ca7aa)

### Step 3: Flash EdgeTX

The following link provides instructions to flash EdgeTX to PL18/PL18EV:
[Flashing EdgeTX to Flysky PL18 or Paladin-EV](flashing-flysky-pl18.md)

### Step 4: (PL18EV Only) Reroute stick ends

Since we have used one of the stick ends buttons for DAC audio output, the original stick ends output need to be rerouted to the RX of UART7.
![Reroute stick end buttons](https://github.com/EdgeTX/edgetx/assets/18414861/591c2c8e-9a5d-4fa4-9d14-2070635d90bf)


### Step 5: (Optional PL18 Only) Install an internal Multi-Protocol Module (MPM)

There exists an open sourced design of an internal 4-in-1 Multi-Protocol-Module (MPM) for Flysky PL18, please see for more details: https://github.com/richardclli/DIY-PL18-InternalMPM

![PL18 Internal MPM](https://github.com/richardclli/DIY-PL18-InternalMPM/blob/main/images/ModuleInstall.jpg)

Have Fun!!!
