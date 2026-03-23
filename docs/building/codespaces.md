# Building in a Web Browser with GitHub CodeSpaces

An easy option to build EdgeTX radio firmwares without having to set up a local build environment on your own computer, is provided by GitHub CodeSpaces. The only requirement is that you have an account at [GitHub](https://github.com/).

# Preparation

To start building EdgeTX firmware for your radio, open the following link with the web browser of your choice:

https://codespaces.new/EdgeTX/edgetx

and you should be greeted with a screen similar to the following. If you don't want to build from the main development branch (i.e. you want to build a from a stable release), you can change to a different branch, i.e. 2.11 (at 1). If you want builds to run faster, you can also change the machine type to one with more CPUs and RAM (at 2), but please note this will reduce the amount of free time you have for using GitHub Codespaces (120 hrs per month for 2 CPU machine).

<img width="820" height="590" alt="image" src="https://github.com/user-attachments/assets/d1954cbe-e1cc-4180-a362-ae3a4acf2d77" />

After a brief wait, you should be greeted with a Visual Studio Code like development environment in a browser with EdgeTX source code tree preloaded from GitHub. It may still take a few minutes to load the first time, as it prepares the build environment. You should see something similar to this at the bottom right while this is underway:

<img width="477" height="86" alt="image" src="https://github.com/user-attachments/assets/1f33760d-8bb6-4536-8e2b-399192ec5c53" />

Once it has completed, you should have a screen similar to this. The file browser (at 1) of the left will let you navigate the file system, and also download your compiled firmware (as described later). The buttons at the top right (at 2) let you turn the various side panels on and off - I would suggest you turn off the right sidebar as it is just cluttering the place. The section at the bottom (at 3) is the terminal, which you will need in order to run the various compile commands that follow.

<img width="800"  alt="image" src="https://github.com/user-attachments/assets/08254aa5-297d-49a1-9d61-4c8621bc17f9" />

As part of preparing the build environment, it should have initialised all the submodules needed to build the firmware for you. If for some reason this hasn't happened, run `git submodule update --init --recursive` at the terminal.

# Building firmware

Firstly, change to (and create if it does not exist) a build directory:
```
mkdir -p build && cd build
```

The next step is to tell the development system for which radio and with which configuration option you would like the firmware to be built. You do this by entering a _cmake_ command in the terminal section of the Visual Studio Code window (bottom right pane; feel free to close all the popups if they come on top). Here for example:

```
cmake -Wno-dev -DPCB=X10 -DPCBREV=TX16S -DBLUETOOTH=YES -DCMAKE_BUILD_TYPE=Release ../
```

we instruct the CMake system to create makefiles for building for the RadioMaster TX16S (PCB=X10, PCBREV=TX16S), enable Bluetooth support (BLUETOOTH=YES) and selected the type as a Release build without debug symbols included (CMAKE_BUILD_TYPE=Release).
(If you are curious about the options available, enter the following command in terminal to list all options: `cmake -LAH ../` )

Next, issue:
```
cmake --build . --target arm-none-eabi-configure --parallel 2
```

!!! note
    The `--parallel 2` in the command above instructs the cmake build system as to how many tasks to execute in parallel. Typically, you would set this to the number of CPU cores your machine has. So if you are using a higher CPU core count GitHub machine type, you can increase this to suit, and get a faster build.

Only a few seconds later, you should be greeted with "-- Generating done" message.

If so, enter again in terminal the following line to start the compilation and linking step and press Enter:
```
cmake --build arm-none-eabi --target firmware
```

This time it can take few minutes so until the firmware binary is successfully built. If you see "[100%] Built target firmware" then all went smoothly, and you have just made yourself a custom EdgeTX firmware. If you are making incremental modifications to the firmware, subsequent firmware builds will be a lot faster.

!!! tip
    If you want more information about the firmware binary - such as how big the firmware is in relation to the FLASH memory the target handset has, you can compile the `firmware-size` target instead of firmware, and you'll get a summary at the end of the firmware build.

It's a good idea to rename the binary, so that it is easier later to see the target radio and which options were baked into it. For this, issue in the terminal:
```
cd arm-none-eabi
mv firmware.bin edgetx_main_tx16s_bt_release.bin
```

In the left file browser tree open the **build** folder and then open **arm-none-eabi** folder. Right click at _edgetx_main_tx16s_bt_release.bin_ and select Download.

Put the downloaded firmware binary into your radio SD card \FIRMWARE subfolder, and flash it to your radio either using EdgeTX bootloader, [EdgeTX Buddy](https://buddy.edgetx.org/), EdgeTX Companion or [STM32CubeProgrammer](../mods/unbrick.md).

Use [EdgeTX Buddy](https://buddy.edgetx.org/) or EdgeTX Companion to fill your SD card appropriately for your radio.
