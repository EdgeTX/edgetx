#!/usr/bin/env bash

set -e

## Bash script to setup EdgeTX development environment on Ubuntu 24.04 running on bare-metal or in a virtual machine.
## Can run as normal user (with sudo) or as root user

# Configuration constants
QT_VERSION="6.9.0"
ARM_TOOLCHAIN_VERSION="14.2.rel1"
QT_INSTALL_DIR="/opt/qt"

PAUSEAFTEREACHLINE="false"
STEP=1
IS_ROOT=false

# Detect system architecture
ARCH=$(uname -m)
case "$ARCH" in
	x86_64)
		ARM_TOOLCHAIN_ARCH="x86_64"
		echo "Detected x86_64 architecture"
		;;
	aarch64|arm64)
		ARM_TOOLCHAIN_ARCH="aarch64"
		echo "Detected ARM64 architecture"
		;;
	*)
		echo "ERROR: Unsupported architecture: $ARCH"
		echo "This script supports x86_64 and aarch64/arm64 only."
		exit 1
		;;
esac

# Parse argument(s)
for arg in "$@"
do
	if [[ $arg == "--pause" ]]; then
		PAUSEAFTEREACHLINE="true"
	fi
done

# Check if running as root
if [[ $EUID -eq 0 ]]; then
	IS_ROOT=true
	SUDO_CMD=""
	echo "Running as root user"
else
	SUDO_CMD="sudo"
	echo "Running as normal user (will use sudo when needed)"
fi

# Check for network connectivity
echo "=== Checking network connectivity ==="
if ! ping -c 1 -W 2 8.8.8.8 &> /dev/null; then
	echo "ERROR: No network connectivity detected!"
	echo "Please check your internet connection and try again."
	exit 1
fi
echo "Network connectivity OK"

# Check Ubuntu version
echo "=== Checking Ubuntu version ==="
if command -v lsb_release &> /dev/null; then
	if [[ $(lsb_release -rs) != "24.04" ]]; then
		echo "ERROR: Not running on Ubuntu 24.04!"
		echo "Terminating the script now."
		exit 1
	fi
else
	# Fallback: check /etc/os-release
	if [[ -f /etc/os-release ]]; then
		source /etc/os-release
		if [[ "$VERSION_ID" != "24.04" ]] || [[ "$ID" != "ubuntu" ]]; then
			echo "ERROR: Not running on Ubuntu 24.04!"
			echo "Terminating the script now."
			exit 1
		fi
	else
		echo "WARNING: Cannot verify Ubuntu version."
		echo "This script is designed for Ubuntu 24.04 and may not work correctly on other systems."
		read -p "Do you want to proceed anyway? (y/N): " -n 1 -r
		echo
		if [[ ! $REPLY =~ ^[Yy]$ ]]; then
			echo "Aborting script."
			exit 1
		fi
	fi
fi
echo "Ubuntu version check passed"

echo "=== Step $((STEP++)): Updating Ubuntu package lists. Please provide sudo credentials, when asked ==="
$SUDO_CMD apt-get -y update
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Installing packages ==="
$SUDO_CMD apt-get -y install \
	build-essential \
	cmake \
	gcc \
	git \
	lib32ncurses-dev \
	lib32z1 \
	libfox-1.6-dev \
	libsdl2-dev \
	software-properties-common \
	wget \
	zip \
	python3-pip-whl \
	python3-pil \
	libgtest-dev \
	python3-pip \
	python3-tk \
	python3-setuptools \
	clang \
	python3-clang \
	libusb-1.0-0-dev \
	stlink-tools \
	openocd \
	npm \
	pv \
	libclang-dev \
	python-is-python3 \
	openssl
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Installing Python packages ==="
# Python 3.11 introduced the managed environment breakage aka PEP 668.
# since we are building a self-contained environment the simple fix is to add --break-system-packages to all pip installs
python3 -m pip install --break-system-packages -U setuptools && \
python3 -m pip install --break-system-packages \
	filelock \
	asciitree \
	jinja2 \
	pillow \
	clang==14.0.0 \
	future \
	lxml \
	lz4 \
	aqtinstall \
	pyelftools
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Installing Qt ==="
$SUDO_CMD mkdir -p ${QT_INSTALL_DIR}
if [[ $IS_ROOT == false ]]; then
	# Allow current user to write to Qt directory to avoid permission issues
	$SUDO_CMD chown $(whoami):$(whoami) ${QT_INSTALL_DIR}
fi
aqt install-qt --outputdir ${QT_INSTALL_DIR} linux desktop ${QT_VERSION} linux_gcc_64 -m qtmultimedia qtserialport
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Fetching GNU Arm Embedded Toolchains ==="
# EdgeTX uses GNU Arm Embedded Toolchain version ${ARM_TOOLCHAIN_VERSION}
wget -q --show-progress --progress=bar:force:noscroll https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_TOOLCHAIN_VERSION}/binrel/arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-${ARM_TOOLCHAIN_ARCH}-arm-none-eabi.tar.xz
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Unpacking GNU Arm Embedded Toolchains ==="
pv arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-${ARM_TOOLCHAIN_ARCH}-arm-none-eabi.tar.xz | tar xJf -
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Removing the downloaded archives ==="
rm arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-${ARM_TOOLCHAIN_ARCH}-arm-none-eabi.tar.xz
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Moving GNU Arm Embedded Toolchains to /opt ==="
$SUDO_CMD mv arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-${ARM_TOOLCHAIN_ARCH}-arm-none-eabi /opt/gcc-arm-none-eabi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Adding GNU Arm Embedded Toolchain to PATH of current user ==="
if [[ $IS_ROOT == true ]]; then
	# For root, add to /etc/profile.d/ so it's available system-wide
	echo 'export PATH="/opt/gcc-arm-none-eabi/bin:$PATH"' | $SUDO_CMD tee /etc/profile.d/gcc-arm-none-eabi.sh > /dev/null
	$SUDO_CMD chmod +x /etc/profile.d/gcc-arm-none-eabi.sh
else
	# For normal user, add to their .bashrc
	if ! grep -q '/opt/gcc-arm-none-eabi/bin' ~/.bashrc; then
		echo 'export PATH="/opt/gcc-arm-none-eabi/bin:$PATH"' >> ~/.bashrc
	fi
fi
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please press Enter to continue or Ctrl+C to stop."
	read
fi

echo "=== Step $((STEP++)): Removing modemmanager (conflicts with DFU) ==="
$SUDO_CMD apt-get -y remove modemmanager
if [[ $PAUSEAFTEREACHLINE == "true" ]]; then
	echo "Step finished. Please check the output above and press Enter to continue or Ctrl+C to stop."
	read
fi

echo "Finished setting up EdgeTX development environment."
echo "Qt has been installed to: ${QT_INSTALL_DIR}"
if [[ $IS_ROOT == false ]]; then
	echo "Please execute: source ~/.bashrc"
else
	echo "Please re-login or execute: source /etc/profile.d/gcc-arm-none-eabi.sh"
fi
