# Building and Debugging with Qt Creator

!!! warning "Obsolete"
    **The info in this page is obsolete and does not work anymore with present EdgeTX versions.** This guide is no longer maintained and is provided for historical reference only. The configuration steps described here were written for much older versions of EdgeTX and Qt Creator.

This page discusses how to setup and use [Qt Creator](https://wiki.qt.io/QtCreatorWhitepaper) graphical integrated development environment (IDE) to explore and also debug EdgeTX source code.

- [Configuring Qt Creator for EdgeTX](#configuring-qt-creator-for-edgetx)
  * [Activating BareMetal plugin](#activating-baremetal-plugin)
  * [Adding GNU ARM compiler and debugger support to Qt Creator](#adding-gnu-arm-compiler-and-debugger-support-to-qt-creator)
  * [Creating a bare metal device and adding a debugging option to it](#creating-a-bare-metal-device-and-adding-a-debugging-option-to-it)
  * [Creating a Kit for building firmware for STM32 µCs](#creating-a-kit-for-building-firmware-for-stm32-cs)
- [Loading EdgeTX source tree into Qt Creator](#loading-edgetx-source-tree-into-qt-creator)
- [Building EdgeTX firmware with Qt Creator](#building-edgetx-firmware-with-qt-creator)
- [Editing source code with Qt Creator](#editing-source-code-with-qt-creator)

[![Qt Creator code completion](../../assets/images/build/qtcreator/QtCodeCompletion.png)](../../assets/images/build/qtcreator/QtCodeCompletion.png)

With [Qt Creator](https://wiki.qt.io/QtCreatorWhitepaper), you not only have a graphical option to build EdgeTX radio firmware, Companion and Simulator software, but also very comfortable way to edit the code and even more importantly to debug it. If you hook up to your radio's [Serial-Wire-Debug (SWD)](https://stm32-base.org/guides/connecting-your-debugger.html) header of the mainboard a hardware debugger, it is possible to perform in-circuit-debugging of code running on the radios STM32 microcontroller. Some examples of hardware debuggers that can be used are, e.g. [Segger J-Link](https://www.segger.com/products/debug-probes/j-link/) (incl. the [Mini EDU](https://www.segger.com/products/debug-probes/j-link/models/j-link-edu-mini/)) and [ST-Link/V2](https://www.st.com/en/development-tools/st-link-v2.html). With in-circuit-debugging, you are able to step through the code and do live variable inspection, which makes validating low level code much easier. For debugging high-level code, it is easier to use the radio firmware simulation library, running on your PC, not requiring a physical radio hardware hookup. Even if you might not be driven by the desire to change the existing EdgeTX codebase, running code in a debug session will provide you an in-depth insight into variables passed between the functions and can in general give a clearer view of how the EdgeTX runs under the hood.

If you followed the previous build instruction guides (irrespective if for Windows 10 or Ubuntu Linux 20.04), Qt Creator IDE is already installed on your system.
The guide below uses screenshots from Windows, but provides info for correct paths in text for Ubuntu Linux as well.

## Configuring Qt Creator for EdgeTX

We need to first configure Qt Creator to support building for ARM architecture and external debugging (so called _bare-metal_ in Qt Creator lingo).

### Activating BareMetal plugin

Launch Qt Creator by clicking on _Start_ -> _Qt_ -> _Qt Creator_. Navigate to menu _Help_ and click _About Plugins..._. Under _Device Support_ enable the _BareMetal (experimental)_ plugin. Click _Close_

[![Qt Creator BareMetal plugin](../../assets/images/build/qtcreator/QtCreatorBareMetal.png)](../../assets/images/build/qtcreator/QtCreatorBareMetal.png)

After this step, it is mandatory to restart Qt Creator, select _Restart Now_ from the pop-up to restart Qt Creator.

### Adding GNU ARM compiler and debugger support to Qt Creator

Open from menu _Tools_ -> _Options..._. In the left pane _Kits_ should already be selected, if not, select it. Open tab _Compilers_. Click _Add_ -> _GCC_ -> _C_. As name, enter for example `GCC C ARM`. Click _Browse..._ behind field _Compiler path_ and navigate and open

 * Windows: `C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\bin\arm-none-eabi-gcc.exe`
 * Linux: `/opt/gcc-arm-none-eabi/bin/arm-none-eabi-gcc`

Field _ABI:_ should have automatically detected _arm-baremetal-generic-elf-32bit_. Click _Apply_.

Next, we add similarly C++ compiler. Click _Add_ -> _GCC_ -> _C++_. As name, enter for example `GCC ARM` and into field _Compiler path_ enter the same _arm-none-eabi-gcc_ binary as in the last step for plain C. Also here, field _ABI:_ should have detected _arm-baremetal-generic-elf-32bit_. Click once more _Apply_. If everything went smoothly, the red exclamation marks should be gone.

[![Qt Creator Compilers](../../assets/images/build/qtcreator/QtCreatorCompilers.png)](../../assets/images/build/qtcreator/QtCreatorCompilers.png)

Navigate to _Debuggers_ tab. Click _Add_, provide as name for example _GDB ARM_ and click _Browse..._ behind field _Path:_

Navigate to:

  * Windows: `C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\bin\arm-none-eabi-gdb-py.exe`
  * Linux: `/opt/gcc-arm-none-eabi/bin/arm-none-eabi-gdb-py`

and click _Open_. If Qt Creator is happy, then it should list in _Type_ and _ABIs_ fields that it found _GDB_ for _arm-baremetal-generic-elf-32bit_. Click _Apply_.

[![Qt Creator Debuggers](../../assets/images/build/qtcreator/QtCreatorDebuggers.png)](../../assets/images/build/qtcreator/QtCreatorDebuggers.png)

### Creating a bare metal device and adding a debugging option to it

If you plan to carry out in-circuit debugging, them install the drivers and software for your debugging hardware before continuing. For Segger J-Link/J-Trace, get the latest [Segger J-Link software and documentation pack](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) and install it. If your debugger stems from another manufacturer, you might want to consider the Open On-Chip Debugger or short [OpenOCD](http://openocd.org/). It supports huge variety of debugger hardware, but is less trivial to setup.

To add in-circuit-debugging to Qt Creator, start by navigating in the left pane of the Options dialog to _Devices_. Open tab _Bare Metal_, click _Add_ and according to your debugger hardware choose either _JLink_ or _OpenOCD_. The example below shows how to setup in-circuit-debugging with Segger J-Link/J-Trace.

click _Browse..._ behind _Executable file_ and open the command line version of J-Link GDB Server binary at:

 * Windows: `C:\Program Files (x86)\SEGGER\JLink\JLinkGDBServerCL.exe`
 * Linux: `/opt/SEGGER/JLink/JLinkGDBServer`

Make sure the _Target interface_ is set to _SWD_ and adjust the speed according to your J-Link hardware (the [following page](https://www.segger.com/products/debug-probes/j-link/models/model-overview/) lists the maximum target interface speed of Segger products). Under _Device:_ use the STM32 chip according to your hardware. For Jumper T16 and RadioMaster TX16S, this is for example _STM32F429BI_.
The remaining settings can be left at default, click _Apply_

[![Qt Creator JLink setup](../../assets/images/build/qtcreator/QtCreatorJLink.png)](../../assets/images/build/qtcreator/QtCreatorJLink.png)

Navigate to tab _Devices_. Click _Add.._, pick _Bare Metal Device_ and click _Start Wizard_. Give it a name, such as `STM32`. Pick a _GDB server provider:_ according to your debugger hardware either _JLink_ or _OpenOCD_. If you do not possess a debugger hardware, you can choose _None_ here. Click _Finish_, click _Apply_.

[![Qt Creator Devices](../../assets/images/build/qtcreator/QtCreatorDevices.png)](../../assets/images/build/qtcreator/QtCreatorDevices.png)

### Creating a Kit for building firmware for STM32 µCs

This step gathers the options for building for STM32 microcontrollers that we have prepared in the last steps.

In the left pane of Qt Creator Options dialog, choose _Kits_ and navigate to similarly named tab _Kits_. Add a new kit by clicking _Add_. Give it a name, like `STM32`. Under _Device type:_ pick _Bare Metal Device_.

If under _Device:_ _STM32 (default for Bare Metal)_ is not already automagically selected, pick it from the drop-down box. In fields _Compiler:_ choose our previously prepared _GCC C ARM_ and _GCC ARM_ options. For field _Debugger:_, choose our prepared _GDB ARM_. As there is no Qt code in our firmware for radio µC, select `None` for the _Qt version_. Click _Apply_.

Next, we need to result to a trick to disable CMake to run a so called _simple test_, when loading a project. This is required to avoid a warning in present cross-compilation situation (cross-compilation here means that we compile for ARM architecture in STM32 microcontrollers under x86/x64 architecture). Click _Change..._ behind _CMake Configuration_ and append the following two lines in _Edit CMake Configuration_ dialog:
```
CMAKE_CXX_COMPILER_WORKS:STRING=1
CMAKE_C_COMPILER_WORKS:STRING=1
```
and click _OK_ for the _Edit CMake Configuration_ dialog. Click _Apply_ in the Qt Creator Options dialog.

[![Qt Creator CMake Configuration](../../assets/images/build/qtcreator/QtCreatorCMakeConfiguration.png)](../../assets/images/build/qtcreator/QtCreatorCMakeConfiguration.png)

Verify that in the Kits list, our newly created _STM32_ kit is selected, then click _Make Default_

[![Qt Creator Kits](../../assets/images/build/qtcreator/QtCreatorKits.png)](../../assets/images/build/qtcreator/QtCreatorKits.png)

Depending on your hardware debugger maximum target speed, flashing STM32 µC in the radio can take some moments. The default wait time in Qt Creator for GNU debugger (GDB) is 20 seconds, which is typically a bit short for STM32 chips with larger memories and results in an unnecessary pop-up. Next, we will prolong the default waiting time. In the left pane of Qt Creator Options dialog click _Debugger_ and navigate to _GDB_ tab. Set _GDB timeout_, e.g. to 60 seconds:

[![Qt Creator GDB timeout](../../assets/images/build/qtcreator/QtGDBtimeout.png)](../../assets/images/build/qtcreator/QtGDBtimeout.png)

Click _OK_ to close the options dialog.

This concludes setting up Qt Creator for EdgeTX.

## Loading EdgeTX source tree into Qt Creator

In the following, we assume that you followed the instructions setting up the build environment and building EdgeTX firmware (for Windows 10 or for Ubuntu 20.04) and have created a dedicated EdgeTX folder (`C:\edgetx` for Windows or `~/edgetx` for Linux) and downloaded or git cloned EdgeTX source under this dedicated folder.

For the sake of clarity, we will create two new folders to put the output of our Qt build. Create two new empty folders:

 * Windows: `C:\edgetx\build-edgetx-qt-fw` and `C:\edgetx\build-edgetx-qt-comp_sim_libsim`
 * Linux: `~/edgetx/edgetx-v2.4/build-qt-fw` and `~/edgetx/edgetx-v2.4/build-qt-comp_sim_libsim`

Back in Qt Creator, let's import next the EdgeTX source files into a project. From Qt Creator menu, choose _File_ -> _Open File or Project..._. Open:

 * Windows: `C:\edgetx\edgetx24\CMakeLists.txt`
 * Linux: `~/edgetx/edgetx-v2.4/CMakeLists.txt`

Next we need to pick the kits for building. If not already automatically selected, pick _STM32_ that is required for building firmware for ARM architecture. Presently, only under Ubuntu, you can also select _Desktop_ to build Companion, Simulator and radio firmware simulator libraries. If there show up any _Imported Kits_, deselect them (typically only happens if you do not open the project for the first time).

Click _Details_ behind _STM32_ to unfold a build type selection dialog. As we are presently only interested in debugging, leave it selected, but you can deselect all other three options (_Release_, _Release with Debug Information_ and _Minimum Size Release_). Click _Browse..._ behind _Debug_ and open the following folder as build output location:

 * Windows: `C:\edgetx\build-edgetx-qt-fw`
 * Linux: `~/edgetx/edgetx-v2.4/build-qt-fw`

If you opted for building also Companion, Simulator and radio firmware simulator libraries under Ubuntu, click _Details_ behind _Desktop_ and also here, leave only _Debug_ selected. Click _Browse..._ and provide `~/edgetx/edgetx-v2.4/build-qt-comp_sim_libsim` as build target location.

[![Qt Creator Project Kit Selection](../../assets/images/build/qtcreator/QtCreatorProjectKitSelection.png)](../../assets/images/build/qtcreator/QtCreatorProjectKitSelection.png)

Click _Configure Project_.

Click on _Projects_ on the left vertical button bar (so called Mode Selector) and verify that _STM32_ -> _Build_ is currently selected.

Qt Creator presents us the build configuration options as a selection list. These match the previous CMake command line options. As an example, we will select to build for RadioMaster TX16S using matching options we listed in the command line build example for Windows 10 and Ubuntu Linux 20.04 pages.

 * Look for DEFAULT -> DEFAULT_MODE and enter 2 into the value field on the right
 * GVARS should be already selected
 * Disable HELI (if you like)
 * LUA -> LUA should already be selected
 * PCB list is presented as a drop-down options list, choose here `X10` for TX16S
 * type in PCBREV field `TX16S` (instead of default 2014)
 * under PPM -> PPM_UNIT, select US from the dropdown list

Please note that _INTERNAL_GPS_ is not listed, so we need to add it manually. Click _Add_, select as type _Boolean_ and type `INTERNAL_GPS` and select it (make the state to ON).

[![Qt Creator CMake Options](../../assets/images/build/qtcreator/QtCreatorCMakeOptions.png)](../../assets/images/build/qtcreator/QtCreatorCMakeOptions.png)

In case you plan performing in-circuit-debugging, you might want to turn off _UNEXPECTED_SHUTDOWN_, in order not to enter EdgeTX _Emergency Mode_ screen on every boot that was not previously nicely shut-down. Be aware though that this has the potential to corrupt the file system on microSD card, thus ideally use a separate card for debugging, or at least make sure the file system is OK, before you go and fly with it.

Click _Apply Configuration Settings_. Please wait for the process to end, it can take some moments.

Under _Build Steps_, click _Details_ to open a selection menu of what to build. Here, we only want to build and debug _firmware_, thus select _firmware_ and then deselect _all_.

[![Qt Creator Build Steps](../../assets/images/build/qtcreator/QtCreatorBuildSteps.png)](../../assets/images/build/qtcreator/QtCreatorBuildSteps.png)

We need to add two environment variables. Under _Build Environment_ click _Details_ to the right of _Use System Environment_ to unfold the variable list.
Click _Add_ and give the variable a name `C_INCLUDE_PATH` with value `C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\arm-none-eabi\include`

Click once more _Add_ to add a second environment variable with a name `CPLUS_INCLUDE_PATH` and a bit longer value `C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\arm-none-eabi\include\c++\10.2.1;C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2020-q4-major\arm-none-eabi\include\c++\10.2.1\arm-none-eabi`

[![Qt Creator Build Environment](../../assets/images/build/qtcreator/QtCreatorBuildEnvironment.png)](../../assets/images/build/qtcreator/QtCreatorBuildEnvironment.png)

Now, we should have everything set up. Let's set our deploy target by clicking in the bottom left of Qt Creator window the chip symbol with OpenTX and Debug labels. Choose _STM32_ as Kit and _firmware (on STM32)_ as Run target:

[![Qt Creator Deploy Target](../../assets/images/build/qtcreator/QtCreatorDeployTarget.png)](../../assets/images/build/qtcreator/QtCreatorDeployTarget.png)

## Building EdgeTX firmware with Qt Creator

To test if building works, click the hammer icon (Build button) on lower left corner to build firmware. On the bottom of Qt Creator window, you can navigate to _4 Compile Output_ and click it to see compiler progress. After some minutes you should have _firmware.bin_ and _firmware.elf_ files built and saved in `C:\edgetx\build-edgetx-qt-fw`

[![Qt Creator Build Completed](../../assets/images/build/qtcreator/QtCreatorBuildCompleted.png)](../../assets/images/build/qtcreator/QtCreatorBuildCompleted.png)

## Editing source code with Qt Creator

To edit the source of EdgeTX, click in Mode Selector _Edit_. Unfold _OpenTX [2.4]_ and open a source file you would like to edit under `radio -> src -> firmware -> Source Files`. Qt Creator offers many wide established features like syntax highlighting, code completion, refactoring, integrated version control and many more.

[![Qt Creator Code Completion](../../assets/images/build/qtcreator/QtCodeCompletion.png)](../../assets/images/build/qtcreator/QtCodeCompletion.png)

For deeper instructions into Qt Creator, please see [Qt Creator Manual](https://doc.qt.io/qtcreator/).

 * You can find the code for radio firmware and libsimulator under `radio/src/firmware/Source Files` (main entry point is in `opentx.cpp`).
 * Companion software is under `companion/src/companion/Source Files` (main entry point is in `companion.cpp`)
 * Simulator source code you find under `companion/src/simulator/Source Files` (main entry point is in `simulator.cpp`)

Please note that Companion, Simulator and libsimulator should be built using the _Desktop kit_ and not _STM32 kit_, as these will run on x86/x64 and not on ARM! Currently building radio simulator library does not yet work under Windows!
