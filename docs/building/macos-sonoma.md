# Build Instructions using macOS 14.1 (Sonoma)

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

### Install Qt 5:
```
brew install qt@5
```

Once Qt has been installed properly, you should set a couple environment variables (please modify according to the real installation paths):
```
export QTDIR=$(brew --prefix)/opt/qt@5
export QT_PLUGIN_PATH=$QTDIR/plugins
```

Please note that `QT_PLUGIN_PATH` is required to be able to run Companion from your build directory without having to build a `DMG` package first.

### Install ARM toolchain

Download and install ARM GCC from here (installs in `/Applications/ARM/`):
- https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain-13.2.rel1-darwin-x86_64-arm-none-eabi.pkg

Please note that this installation takes care of allowing the binaries to be run and "un-quarantines" them. If you choose to install the `bz2` archive to another location, you will have to take care of that yourself (see https://disable-gatekeeper.github.io/ for more details).

### Install various dependencies

- With `brew` in a `Terminal`:
```
brew install sdl fox cmake
```

If you plan to run the standalone simulator for debuging
```
brew install --cask quartz
```

- Install Python dependencies:
```
pip3 install Pillow clang lz4 jinja2
```

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
   -DARM_TOOLCHAIN_DIR=/Applications/ArmGNUToolchain/13.2.Rel1/arm-none-eabi/bin/ \
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

## Notes on possible error while trying to run build-companion.sh

If you encounter this error:
```
CPack: - Install project: EdgeTX []
CMake Error at /opt/homebrew/Cellar/cmake/3.27.7/share/cmake/Modules/BundleUtilities.cmake:458 (message):
  otool -l failed: 1

  error:
  /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/otool-classic:
  can't open file: @rpath/libsharpyuv.0.dylib (No such file or directory)

Call Stack (most recent call first):
  /opt/homebrew/Cellar/cmake/3.27.7/share/cmake/Modules/BundleUtilities.cmake:527 (get_item_rpaths)
  /opt/homebrew/Cellar/cmake/3.27.7/share/cmake/Modules/BundleUtilities.cmake:614 (set_bundle_key_values)
  /opt/homebrew/Cellar/cmake/3.27.7/share/cmake/Modules/BundleUtilities.cmake:933 (get_bundle_keys)
  /Users/jean-christophedreyfus/Documents/edgetx/build/native/companion/src/cmake_install.cmake:180 (fixup_bundle)
  /Users/jean-christophedreyfus/Documents/edgetx/build/native/cmake_install.cmake:43 (include)


CPack Error: Error when generating package: companion
make: *** [package] Error 1
```

It can be fixed by entering the following command at terminal:

`install_name_tool -change "@rpath/libsharpyuv.0.dylib" "$(brew --prefix)/lib/libsharpyuv.0.dylib" "$(brew --prefix)/lib/libwebp.7.dylib" && codesign --force -s - "$(brew --prefix)/lib/libwebp.7.dylib"`
