# Build Instructions under Windows

Note: The instructions on this page apply only for building V3.0.0-dev and later versions of EdgeTX.

## Table of Contents

* [Toolchain setup](#toolchain-setup)
  1. [Visual Studio](#visual-studio)
  2. [ARM GNU Toolchain](#arm-gnu-toolchain)
  3. [Folders for libraries of the project](#folders-for-libraries-of-the-project)
  4. [Build script](#build-script)
  5. [NSIS Windows installer creator](#nsis-windows-installer-creator)
  6. [Python package manager uv](#python-package-manager-uv)
  7. [Python virtual environment](#python-virtual-environment)
  8. [Qt development framework](#qt-development-framework)
* [Building](#building)
  1. [Fetch the EdgeTX project](#fetch-the-edgetx-project)
  2. [Target specific build script](#target-specific-build-script)

***

## Toolchain setup

### Visual Studio

Download and start Visual Studio 2022 Community installer: https://visualstudio.microsoft.com/downloads/

To start the installer in English to more easily follow the selection below, open a command prompt and add `--locale en-US` to the installer executable when starting it. Under `Individual components` tab, minimally select the following components:
```
  Under Code tools:
    Git for Windows
  Under Compilers, build tools and runtimes:
    C++ Clang Compiler for Windows (19.1.5)
    C++ CMake tools for Windows
    MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest) (is typically already selected by default)
  Under SDKs, libraries, and frameworks:
    Windows 11 SDK (10.0.26100.4654)
```

<img width="475" src="https://github.com/user-attachments/assets/f259b126-e1cf-43c2-883b-834993428769" />

In the a dialog asking if you want to continue without workloads, you can accept just to `Continue`. You can skip making an account and logging in, if asked by the installer.

### ARM GNU Toolchain

Download https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-mingw-w64-x86_64-arm-none-eabi.exe

Install it to default location and select Add path to environment variable.

<img width="480" src="https://github.com/user-attachments/assets/c5d6f6e2-fe70-4b91-8c23-f69b9ba6d45d" />

### Folders for libraries of the project

Create a folder to hold the development environment and the project, e.g. `C:\edgetx`.

Create a subfolder `C:\edgetx\build-tools`.

Download https://github.com/libsdl-org/SDL/releases/download/release-2.32.8/SDL2-devel-2.32.8-VC.zip and unpack it to `C:\edgetx\build-tools\SDL2`

### Build script

Create `C:\edgetx\edgetx_build.cmd` with following contents:

```
@echo off

time /t

set CC=clang.exe
set CXX=clang++.exe

set NSIS_EXE=c:\Program Files (x86)\NSIS\makensis.exe
set SDL2=%EDGETX_BUILD_TOOLS%\SDL2\cmake
set QT_DIR=%EDGETX_BUILD_TOOLS%\Qt\6.9.0\msvc2022_64
set PATHS="-DSDL2_DIR=%SDL2%"
set CMAKE_PREFIX_PATH="-DCMAKE_PREFIX_PATH=%QT_DIR%"
set LANGUAGE="-DTRANSLATIONS=%BUILD_LANGUAGE%"

echo ********************************************
echo Building radio: %BUILD_RADIO%
echo Language:       %BUILD_LANGUAGE%
echo Debug build:    %BUILD_DEBUG%
echo Firmware:       %BUILD_FW%
echo Companion:      %BUILD_CPN%
echo Installer:      %BUILD_INSTALLER%
echo ********************************************

REM
REM prepare build
REM
cd %EDGETX_REPO%

if %BUILD_DEBUG%=="yes" (
  set BUILD_TYPE="-DDEBUG=YES;-DCMAKE_BUILD_TYPE=Debug"
) else (
  set BUILD_TYPE="-DCMAKE_BUILD_TYPE=Release"
)

cd %EDGETX_MAIN_DIR%
set BUILD_FOLDER="build-output-%BUILD_RADIO%"
mkdir %BUILD_FOLDER%
cd %BUILD_FOLDER%
call "c:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -startdir=none -arch=x64 -host_arch=x64
call "%EDGETX_BUILD_TOOLS%\.venv\Scripts\activate.bat"

REM
REM start build
REM
cmake -G Ninja %BUILD_OPTIONS% %PATHS% %CMAKE_PREFIX_PATH% %LANGUAGE% %BUILD_TYPE% %EDGETX_REPO%

if %BUILD_FW%=="yes" (
  ninja arm-none-eabi-configure
  ninja -C arm-none-eabi firmware
)

REM If building installer is requested, need to build Companion
if %BUILD_INSTALLER%=="yes" GOTO build_cpn
if %BUILD_CPN%=="yes" GOTO build_cpn

REM else
GOTO exit_script

:build_cpn
ninja native-configure
ninja -C native libsimulator
ninja -C native simulator
ninja -C native companion

if %BUILD_INSTALLER%=="yes" (
  ninja -C native install
  %QT_DIR%\bin\windeployqt --libdir native\_install\bin --plugindir native\_install\plugins --no-compiler-runtime --no-translations --release native
  robocopy %QT_DIR%\bin\ native\_install\bin qt.conf
  "%NSIS_EXE%" native/companion/companion.nsi
)

REM The following is in order to be able to run companion and simulator binaries directly in build output folder
%QT_DIR%\bin\windeployqt --libdir native --plugindir native\plugins --no-compiler-runtime --no-translations --release native
copy %EDGETX_BUILD_TOOLS%\SDL2\lib\x64\SDL2.dll %EDGETX_MAIN_DIR%\%BUILD_FOLDER%\native\

REM
REM done
REM
:exit_script
call "%EDGETX_BUILD_TOOLS%\.venv\Scripts\deactivate.bat"
time /t
pause
```

### NSIS Windows installer creator

Install the NSIS compiler from https://nsis.sourceforge.io/Download

<img width="480" src="https://github.com/user-attachments/assets/bb8bf3ac-10a1-4bee-b65e-7f914acb79c0" />

### Python package manager uv

Open a command prompt and navigate to `C:\edgetx\build-tools`.

Install Python package manager uv according to the installation instructions at: https://docs.astral.sh/uv/getting-started/installation/#__tabbed_1_2

### Python virtual environment

Continue on the command prompt at `C:\edgetx\build-tools`:
```
uv venv
uv pip install clang lz4 jinja2 pillow pyelftools aqtinstall pydantic
```

### Qt development framework

Continue on the command prompt at `C:\edgetx\build-tools` to install Qt 6.9.0 into the Python virtual environment:
```
mkdir Qt
cd Qt
call "C:\edgetx\build-tools\.venv\Scripts\activate.bat"
aqt install-qt windows desktop 6.9.0 win64_msvc2022_64 -m qtmultimedia qtserialport
call "C:\edgetx\build-tools\.venv\Scripts\deactivate.bat"
```
Reboot your computer.


## Building

### Fetch the EdgeTX project

Open command prompt at `C:\edgetx` and clone the EdgeTX repo:
```
git clone --recursive -b <branch> https://github.com/EdgeTX/edgetx.git
```
where <branch> is the branch or tag you wish to use, e.g. `main` for the main development brach, or use a tag, if you wish to download a release version, such as `v3.0.1`. This will fetch the project into `C:\edgetx\edgetx`.

### Target specific build script

Create a radio specific build script, e.g. here for example for RadioMaster TX16s (create it e.g. as `C:\edgetx\build_tx16s.cmd`):

```
@echo off

set EDGETX_MAIN_DIR=C:\edgetx
set EDGETX_REPO=C:\edgetx\edgetx
set EDGETX_BUILD_TOOLS=C:\edgetx\build-tools

set BUILD_RADIO="TX16s"
set BUILD_OPTIONS="-DPCB=X10;-DPCBREV=TX16S"
set BUILD_LANGUAGE="EN"
set BUILD_DEBUG="no"
set BUILD_FW="yes"
set BUILD_CPN="yes"
set BUILD_INSTALLER="yes"

call "%EDGETX_MAIN_DIR%\edgetx_build.cmd"
```
and start it.

After the build finishes, you can find the binaries under `C:\edgetx\build-output-<BUILD_RADIO>` (firmware binary under subfolder `arm-none-eabi`, Companion and Simulator binaries together with simulator library under folder `native` and Companion installer under subfolder `native\companion`).
Note! Be sure to wipe the `C:\edgetx\build-output-<BUILD_RADIO>` content every time you change the build options.

See https://github.com/EdgeTX/edgetx/blob/main/tools/build-common.sh for BUILD_OPTIONS to use for various EdgeTX supported radios. Note that whereas the shell script at GitHub has spaces as delimeters between the keys, then in the Windows batch file, you neet to use semicolon as a delimeter.
