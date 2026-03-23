# Build Instructions using macOS 15 (Sequoia) and macOS 26 (Tahoe)

# Install [Homebrew](https://brew.sh/)

- Run command in `Terminal`:
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
!!! tip
    Installing Brew via the command above will automatically install the [Xcode Command Line Tools](https://mac.install.guide/commandlinetools/). If for some reason you need to do this manually, run `xcode-select —install` via the Terminal app.

# Install Qt 6
!!! note
    If you only intend on building the firmware, and not `simu`, `companion` or `simulator`, this is not necessary, and you can skip to the next step.

```
brew install qt@6
```

Once Qt has been installed, you should set a couple environment variables (please modify according to the real installation paths):
```
export QTDIR=$(brew --prefix)/opt/qt@6
export QT_PLUGIN_PATH=$QTDIR/plugins
```

Please note that `QT_PLUGIN_PATH` is required to be able to run Companion from your build directory without having to build a `DMG` package first.

# Install ARM toolchain

Download and install the ARM GCC toolchain [from here](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) (installs in `/Applications/ArmGNUToolchain/`):

- For Intel Mac: https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-darwin-x86_64-arm-none-eabi.pkg
- If Mac Silicon (i.e. M1-M5): https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-darwin-arm64-arm-none-eabi.pkg

Please note that this installation takes care of allowing the downloaded binaries to be run and prevents them being quarantined. If you choose to install the `tar.xz` archive to another location, you will have to take care of that yourself (see https://disable-gatekeeper.github.io/ for more details).

# Install various dependencies

- With `brew` in a `Terminal`:
```
brew install sdl cmake
```

If you plan to run the standalone simulator for debugging:
```
brew install --cask quartz
```


# Download EdgeTX code

- Checkout code
```
git clone --recursive https://github.com/EdgeTX/edgetx.git
```

- Switch into the source directory:
```
cd edgetx
```

# Install Python dependencies

Since Python 3.11 enabled the "externally managed" flag, it is recommended that you use a virtual environment. [uv](https://docs.astral.sh/uv/getting-started/installation/) is one of the easiest tools to do and manage this, and can be installed with brew. It is recommended you create the virtual environment now rather than earlier in the process, as doing it now will create it in the edgetx directory.

- Install UV:
```
brew install uv
```

- Create the virtual environment (and use specific version of python for this environment):
```
uv venv --python 3.14
```

- Activate the virtual environment (you will need to run this whenever you want to compile in the future from a new terminal session):
```
source .venv/bin/activate
```

- Install the packages:
```
uv pip install Pillow clang lz4 jinja2
```

# Compile EdgeTX

- Create and enter build directory:
```
mkdir -p build && cd build
```

Configure build flags using `cmake` (in this case, for RadioMaster TX16S, [see here](https://github.com/EdgeTX/edgetx/blob/main/tools/build-common.sh) for other possible handset specific flags).
```
cmake -DPCB=X10 -DPCBREV=TX16S \
   -DCMAKE_PREFIX_PATH=$QTDIR \
   -DARM_TOOLCHAIN_DIR=/Applications/ArmGNUToolchain/14.2.Rel1/arm-none-eabi/bin/ ..
```

!!! note
    Please note that the variables `CMAKE_PREFIX_PATH`, `ARM_TOOLCHAIN_DIR` must be specified additionally to what is described in the other compilation HowTos:

    - `CMAKE_PREFIX_PATH`: this must point to your Qt installation path.
    - `ARM_TOOLCHAIN_DIR`: this must point to where ARM GCC has been installed (and MUST contain `/` at the end).

Configure the compiler for firmware building (parallel limits the number of CPU cores used - you can increase this if your machine can handle more):
```
cmake --build . --target arm-none-eabi-configure --parallel 4
```

Build the firmware!
```
cmake --build . --target firmware
```

# Troubleshooting

## Notes on compiling simulator plug-ins

When compiling simulator plug-ins (using `cmake --build . --target libsimulator` with the target properly configured), the product of this compilation will be a `.dylib` stored in your build directory. If you want the Companion or Simulator apps to be able to use it, you will need to copy it manually into the respective directories. Here is how it should look with a couple of plug-ins copied:

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
