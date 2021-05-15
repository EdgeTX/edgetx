#! /usr/bin/env bash

## Bash script to setup EdgeTX v2.4 development environment on Ubuntu 20.04.
## Let it run as normal user and when asked, give sudo credentials

PAUSEAFTEREACHLINE="false"

# Parse argument(s)
for arg in "$@"
do
	if [[ $arg == "--pause" ]]; then
		PAUSEAFTEREACHLINE="true"
	fi
done

if 
 [[ `lsb_release -rs` != "20.04" ]]; then
  echo "ERROR: Not running on Ubuntu 20.04!"
  echo "Terminating the script now."
  exit 1
fi

echo "=== Step 1: Updating Ubuntu package lists. Please provide sudo credentials, when asked ==="
sudo apt-get -y update
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 2: Installing packages ==="
sudo apt-get -y install build-essential cmake gcc git lib32ncurses-dev lib32z1 libfox-1.6-dev libsdl1.2-dev qt5-default qtmultimedia5-dev qttools5-dev qttools5-dev-tools qtcreator libqt5svg5-dev software-properties-common wget zip python-pip-whl python-pil libgtest-dev python3-pip python3-tk python3-setuptools clang-7 python-clang-7 libusb-1.0-0-dev stlink-tools openocd libncurses5:i386 libpython2.7:i386
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 3: Creating symbolic link for Python ==="
sudo ln -sf /usr/bin/python3 /usr/bin/python
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 4: Installing Python packages ==="
sudo python3 -m pip install filelock pillow clang future lxml
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 5: Fetching GNU Arm Embedded Toolchains ==="
# EdgeTX uses GNU Arm Embedded Toolchain in version 6-2017-q2
wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/6-2017q2/gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 6: Unpacking GNU Arm Embedded Toolchains ==="
tar xjf gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 7: Removing the downloaded archives ==="
rm gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 8: Moving GNU Arm Embedded Toolchains to /opt ==="
sudo mv gcc-arm-none-eabi-6-2017-q2-update /opt/gcc-arm-none-eabi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 9: Adding GNU Arm Embedded Toolchain to PATH of current user ==="
echo 'export PATH="/opt/gcc-arm-none-eabi/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 10: Removing modemmanager (conflicts with DFU) ==="
sudo apt-get -y remove modemmanager
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 11: Fetching USB DFU host utility ==="
wget http://dfu-util.sourceforge.net/releases/dfu-util-0.9.tar.gz
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 12: Unpacking USB DFU host utility ==="
tar xzvf dfu-util-0.9.tar.gz
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 13: Building and Installing USB DFU host utility ==="
cd dfu-util-0.9/
./configure 
make
sudo make install
cd ..
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step 14: Removing the downloaded archive and build folder of USB DFU host utility ==="
rm dfu-util-0.9.tar.gz
rm -rf dfu-util-0.9
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished."
fi

echo "Finished setting up EdgeTX v2.4 development environment. Please reboot the computer."
