# Build Instructions using macOS 10.15 (Catalina)

### Install Xcode command line tools:

- Open `Terminal`
- Run this command:
```
xcode-select —install
```

### Install "brew" (https://brew.sh):

- Run command in `Terminal`:
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Install newer Python version:

- Run command in `Terminal`:
```
brew install python
```

### Install Qt 5.12.9:

- Install `aqtinstall` Python package:

```
pip3 install aqtinstall
```

- Install Qt 5.12.9
```
aqt install 5.12.9 mac desktop clang_64
```

Once Qt has been installed properly, you should set a couple environment variables (please modify according to the real installation paths):
```
export QTDIR=/Users/etx/Src/Qt/5.12.9/clang_64
export QT_PLUGIN_PATH=$QTDIR/plugins
```

Please note that `QT_PLUGIN_PATH` is required to be able to run Companion from your build directory without having to build a `DMG` package first.

### Install ARM toolchain

Download and install ARM GCC from here (installs in `/Applications/ARM/`):
- https://developer.arm.com/-/media/Files/downloads/gnu-rm/10-2020q4/gcc-arm-none-eabi-10-2020-q4-major-mac.pkg

Please note that this installation takes care of allowing the binaries to be run and "un-quarantines" them. If you choose to install the `bz2` archive to another location, you will have to take care of that yourself (see https://disable-gatekeeper.github.io/ for more details).

### Install various dependencies

- With `brew` in a `Terminal`:
```
brew install sdl fox cmake
```

- Install Python dependencies:
```
pip3 install Pillow clang lz4 jinja2
```

- If you face issues from pip3 command, try to fix your CommandLineTools installation
From the terminal run:
```
softwareupdate --list
```
which produces a list of available updates. Wait a bit for a list to display (won't take very long). And look for the "* Label:" under Software Update found the following new or updated software:

It should say something like: * Label: `Command Line Tools for Xcode-13.2`

Then simply run:
```
softwareupdate -i "Command Line Tools for Xcode-13.2"
```
and replace the text in the brackets with the Label from the previous output. This will then install the updates and the fix for python3.

### Compile EdgeTX

- Checkout code
```
git clone --recursive https://github.com/EdgeTX/edgetx.git
```

- Switch into the source directory:
```
cd edgetx
```

- Create build directory and configure build using `cmake`:
```
mkdir -p build

cd build

cmake -DPCB=X10 -DPCBREV=TX16S \
   -DCMAKE_PREFIX_PATH=$QTDIR \
   -DARM_TOOLCHAIN_DIR=/Applications/ARM/bin/ \
   -DPYTHON_EXECUTABLE=$(brew --prefix)/bin/python3 ..
```

Please note that the variables `CMAKE_PREFIX_PATH`, `ARM_TOOLCHAIN_DIR` and `PYTHON_EXECUTABLE` must be specified additionally to what is described in the other compilation HowTos:
- `CMAKE_PREFIX_PATH`: this must point to your Qt installation path.
- `ARM_TOOLCHAIN_DIR`: this must point to where ARM GCC has been installed (and MUST contain `/` at the end).
- `PYTHON_EXECUTABLE`: this allows overriding Python installed as part of MacOS.

- Then build as usual (`-j4` to use 4 CPU cores):
```
make -j4 firmware
```

## Notes on compiling simulator plug-ins

When compiling simulator plug-ins (using `make libsimulator` with the target properly configured), the product of this compilation will be a `.dylib` stored in your build directory. If you want the Companion or Simulator Apps to be able to use it, you will need to copy it manually into the respective directories. Here is how it show look like with a couple plug-ins copied:

```
% ls -l companion.app/Contents/MacOS/
total 411976
-rwxr-xr-x  1 etx  staff  27908864 Jan  7 11:48 companion
-rwxr-xr-x  1 etx  staff  57089384 Jan  7 11:22 libedgetx-nv14-simulator.dylib
-rwxr-xr-x  1 etx  staff  61167768 Jan  7 08:44 libedgetx-tx16s-simulator.dylib
-rwxr-xr-x  1 etx  staff  60327272 Jan  5 12:48 libedgetx-x10express-simulator.dylib
-rwxr-xr-x  1 etx  staff   1849768 Dec 26 08:56 libedgetx-x9d+-simulator.dylib
-rwxr-xr-x  1 etx  staff   1881224 Dec 31 16:44 libedgetx-zorro-simulator.dylib

% ls -l simulator.app/Contents/MacOS/
total 392616
-rwxr-xr-x  1 etx  staff  57089384 Jan  7 11:22 libedgetx-nv14-simulator.dylib
-rwxr-xr-x  1 etx  staff  61168056 Jan  5 11:52 libedgetx-tx16s-simulator.dylib
-rwxr-xr-x  1 etx  staff  60327272 Jan  5 12:34 libedgetx-x10express-simulator.dylib
-rwxr-xr-x  1 etx  staff  21732144 Jan  7 11:48 simulator
```
