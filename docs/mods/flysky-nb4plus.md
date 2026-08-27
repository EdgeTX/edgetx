# Flysky NB4+ Hardware Mod for Complete EdgeTX Support

With some hardware modifications, it is possible to run a full featured EdgeTX firmware on Flysky NB4+ radio. Natively, these radios have the following limitations with EdgeTX:

1. Similar to PL18/PL18EV, NB4+ has limited internal memory that is not big enough to carry a full SD card content. It comes with a 8 MByte sized NOR flash chip, which is sufficient for EdgeTX configuration files, but not for a full EdgeTX experience.
2. Similar to PL18/PL18EV, NB4+ cannot keep by default the date and the time. The internal printed circuit boards are prepared for real-time-clock functionality, but there are a few components are not populated by default. For this reason, RTC functionality will not work properly with EdgeTX out of the box.

Below you can find step-by-step tips of how to modify your NB4+ to overcome the above mentioned problems. Be aware that warranty will be most certainly lost when following the modification steps listed here.

### Step 1: Install RTC battery and upgrade the flash chip

Open up your radio and take out the MCU main PCB that is located right under the LCD display. The MCU PCB is shown here:

![NB4+ MCU PCB 1](https://github.com/EdgeTX/edgetx/assets/18414861/6b7e2acb-927a-4666-a22b-b08dd9593537)

The BLUE SQUARE denotes the NOR flash chip. The factory chip populated is of type W25Q64JV (8MB in size) and is not big enough to fit all required files of the EdgeTX SD card content. You can replace it with MX25L25645G that has the same form factor and pinout, but is 32MB in size. Thus, desolder the W25Q64JV and replace it with MX25L25645G. Be sure to orient the IC the same way when resoldering (match pin 1 location). Besides the choice of a 32MB flash chip, the chips that uses a WSON8 packaging can fit for replacement as well. However this packaging is better soldered using a hot blower instead of a soldering iron and can be an option to those who are skillful enough. Both W25Q512JV (64MB) and W25Q01JV (128MB) NOR flash chips are supported.

The RED SQUARE denotes the location of the RTC battery. This can be populated with MS-621 rechargeable battery. Please note that after installation, it needs some time for it to charge up, before RTC functionality starts to work properly.

In order to have the RTC working properly, only adding the RTC battery is not enough. When you flip the PCB to the other side, there are some components missing for RTC to work properly:

![NB4+ MCU PCB 2](https://github.com/EdgeTX/edgetx/assets/18414861/f95e841d-2730-4adc-b06d-a730ab5716df)

The RED SQUARE denotes the location of the RTC clocking circuit, you need to solder a 32kHz crystal and 2 x 3.9pF capacitors in this location. The YELLOW SQUARE denotes the location of a diode that enable the charging of the RTC battery, just solder a LL4148 diode will works. The BLUE SQUARE denotes the location of 2 capacitors: 100nF and 10pF. You may refer to the following schematic for details about the RTC battery design.

![RTC Battery Schematic](https://github.com/EdgeTX/edgetx/assets/18414861/33363d9e-4c77-45ec-ba77-4a56fb3ddafa)

### Step 2: Flash EdgeTX

The following link provides instructions to flash EdgeTX to PL18/PL18EV:
[Flashing EdgeTX to Flysky PL18 or Paladin-EV](flashing-flysky-pl18.md)

Flashing NB4+ uses the same procedure, only differences are the location of DFU and hard power off buttons:

![NB4+ DFU button](https://github.com/user-attachments/assets/675c7877-ecf7-4015-b642-c5c2ff8b6c5c)

![NB4+ hard power off button](https://github.com/user-attachments/assets/ee5157b3-d38d-4321-a556-a9f2e4f7da8b)

I am still working a way to have usable TX modules with NB4+, stay tune and wait for my update.
