#! /usr/bin/env bash

set -e

## Bash script to setup EdgeTX development environment on Ubuntu 24.04 running on bare-metal or in a virtual machine.
## Let it run as normal user and when asked, give sudo credentials

QT_VERSION="6.9.3"
GCC_ARM_VERSION="14.2.rel1"

PAUSEAFTEREACHLINE="false"
STEP=1
# Parse argument(s)
for arg in "$@"
do
	if [[ $arg == "--pause" ]]; then
		PAUSEAFTEREACHLINE="true"
	fi
done

if [[ $(lsb_release -rs) != "24.04" ]]; then
  echo "ERROR: Not running on Ubuntu 24.04!"
  echo "Terminating the script now."
  exit 1
fi

echo "=== Step $((STEP++)): Setting up package repositories ==="
sudo apt-get -y install --no-install-recommends software-properties-common gpg gpg-agent wget ca-certificates
sudo mkdir -p /etc/apt/keyrings
# Set up Kitware repository for newer cmake
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | \
    gpg --dearmor - | sudo tee /etc/apt/keyrings/kitware-archive-keyring.gpg >/dev/null
echo "deb [signed-by=/etc/apt/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ noble main" | \
    sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
# Set up NodeSource repository for Node.js 20.x
wget -O - https://deb.nodesource.com/gpgkey/nodesource-repo.gpg.key 2>/dev/null | \
    gpg --dearmor - | sudo tee /etc/apt/keyrings/nodesource.gpg >/dev/null
echo "deb [signed-by=/etc/apt/keyrings/nodesource.gpg] https://deb.nodesource.com/node_20.x nodistro main" | \
    sudo tee /etc/apt/sources.list.d/nodesource.list >/dev/null
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
sudo apt-get -y install --no-install-recommends \
    build-essential \
    cmake \
    kitware-archive-keyring \
    git \
    zip \
    unzip \
    file \
    gawk \
    libsdl2-dev \
    python3-dev \
    python3-pip \
    python3-setuptools \
    python3-tk \
    libcups2 \
    libssl-dev \
    libgtest-dev \
    clang \
    libclang-dev \
    python-is-python3 \
    dfu-util \
    nodejs \
    stlink-tools \
    openocd \
    pv
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing lv_font_conv ==="
sudo npm i lv_font_conv -g
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Python packages ==="
# Python 3.12+ enforces PEP 668 (managed environment); --break-system-packages allows pip installs outside a venv
sudo python3 -m pip install --break-system-packages --ignore-installed \
    asciitree \
    jinja2 \
    pillow \
    clang \
    lz4 \
    aqtinstall \
    pyelftools \
    pydantic
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Installing Qt ==="
sudo mkdir -p /opt/qt
sudo chown "${USER}:${USER}" /opt/qt
aqt install-qt --outputdir /opt/qt linux desktop ${QT_VERSION} linux_gcc_64 -m qtmultimedia qtserialport
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Fetching GNU Arm Embedded Toolchains ==="
wget -q --show-progress --progress=bar:force:noscroll https://developer.arm.com/-/media/Files/downloads/gnu/${GCC_ARM_VERSION}/binrel/arm-gnu-toolchain-${GCC_ARM_VERSION}-x86_64-arm-none-eabi.tar.xz
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Unpacking GNU Arm Embedded Toolchains ==="
pv arm-gnu-toolchain-${GCC_ARM_VERSION}-x86_64-arm-none-eabi.tar.xz | tar xJf -
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Removing the downloaded archives ==="
rm arm-gnu-toolchain-${GCC_ARM_VERSION}-x86_64-arm-none-eabi.tar.xz
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Moving GNU Arm Embedded Toolchains to /opt ==="
sudo mv arm-gnu-toolchain-${GCC_ARM_VERSION}-x86_64-arm-none-eabi /opt/gcc-arm-none-eabi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
  echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
  read
fi

echo "=== Step $((STEP++)): Adding GNU Arm Embedded Toolchain and Qt to PATH of current user ==="
cat >> ~/.bashrc << EOF
export PATH="/opt/gcc-arm-none-eabi/bin:\$PATH"
export PATH="/opt/qt/${QT_VERSION}/gcc_64/bin:\$PATH"
EOF
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

echo "Finished setting up EdgeTX development environment."
echo "Please execute: source ~/.bashrc"
