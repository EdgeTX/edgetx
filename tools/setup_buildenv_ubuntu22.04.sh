#! /usr/bin/env bash

set -e

## Bash script to setup EdgeTX development environment on Ubuntu 22.04 running on bare-metal or in a virtual machine.
## Let it run as normal user and when asked, give sudo credentials

PAUSEAFTEREACHLINE="false"
STEP=1
# Parse argument(s)
for arg in "$@"
do
	if [[ $arg == "--pause" ]]; then
		PAUSEAFTEREACHLINE="true"
	fi
done

if [[ $(lsb_release -rs) != "22.04" ]]; then
  echo "ERROR: Not running on Ubuntu 22.04!"
  echo "Terminating the script now."
  exit 1
fi

echo "=== Step $((STEP++)): Checking if i386 requirement is satisfied ==="
OUTPUT=x$(dpkg --print-foreign-architectures 2> /dev/null | grep i386) || :
if [ "$OUTPUT" != "xi386" ]; then
    echo "Need to install i386 architecture first."
    sudo dpkg --add-architecture i386
else
    echo "i386 requirement satisfied!"
fi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Updating Ubuntu package lists. Please provide sudo credentials, when asked ==="
sudo apt-get -y update
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing packages ==="
sudo apt-get -y install build-essential cmake gcc git lib32ncurses-dev lib32z1 libfox-1.6-dev libsdl2-dev qtbase5-dev qt5-qmake qtmultimedia5-dev qttools5-dev qttools5-dev-tools qtcreator libqt5svg5-dev libqt5serialport5-dev software-properties-common wget zip python3-pip-whl python3-pil libgtest-dev python3-pip python3-tk python3-setuptools clang python3-clang libusb-1.0-0-dev stlink-tools openocd npm pv libncurses5:i386 libpython2.7:i386 libclang-dev python-is-python3
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python packages ==="
sudo python3 -m pip install filelock asciitree jinja2 pillow==7.2.0 clang==14.0.0 future lxml lz4
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Fetching GNU Arm Embedded Toolchains ==="
# EdgeTX uses GNU Arm Embedded Toolchain in version 10-2020-q4
wget -q --show-progress --progress=bar:force:noscroll https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Unpacking GNU Arm Embedded Toolchains ==="
pv gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2 | tar xjf -
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Removing the downloaded archives ==="
rm gcc-arm-none-eabi-10-2020-q4-major-x86_64-linux.tar.bz2
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Moving GNU Arm Embedded Toolchains to /opt ==="
sudo mv gcc-arm-none-eabi-10-2020-q4-major /opt/gcc-arm-none-eabi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Adding GNU Arm Embedded Toolchain to PATH of current user ==="
echo 'export PATH="/opt/gcc-arm-none-eabi/bin:$PATH"' >> ~/.bashrc
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Removing modemmanager (conflicts with DFU) ==="
sudo apt-get -y remove modemmanager
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Fetching USB DFU host utility ==="
wget -q --show-progress --progress=bar:force:noscroll http://dfu-util.sourceforge.net/releases/dfu-util-0.11.tar.gz
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Unpacking USB DFU host utility ==="
pv dfu-util-0.11.tar.gz | tar xzf -
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Building and Installing USB DFU host utility ==="
cd dfu-util-0.11/
./configure 
make
sudo make install
cd ..
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Removing the downloaded archive and build folder of USB DFU host utility ==="
rm dfu-util-0.11.tar.gz
rm -rf dfu-util-0.11
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished."
fi

echo "Finished setting up EdgeTX development environment."
echo "Please execute: source ~/.bashrc"
