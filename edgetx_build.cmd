@echo off
set CC=clang.exe
set CXX=clang++.exe
set NSIS_EXE=c:\Program Files (x86)\NSIS\makensis.exe
set SDL2=%EDGETX_BUILD_TOOLS%\SDL2\cmake
set QT_DIR=%EDGETX_BUILD_TOOLS%\Qt\6.9.0\msvc2022_64
set PATHS="-DSDL2_DIR=%SDL2%"
set CMAKE_PREFIX_PATH="-DCMAKE_PREFIX_PATH=%QT_DIR%"
set LANGUAGE="-DTRANSLATIONS=%BUILD_LANGUAGE%"

cd %EDGETX_REPO%
set BUILD_TYPE="-DCMAKE_BUILD_TYPE=Release"

cd %EDGETX_MAIN_DIR%
set BUILD_FOLDER=build-output-%BUILD_RADIO%
if not exist "%BUILD_FOLDER%" mkdir "%BUILD_FOLDER%"
cd "%BUILD_FOLDER%"

if "%VSCMD_VER%"=="" call "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat" -startdir=none -arch=x64 -host_arch=x64
call "%EDGETX_BUILD_TOOLS%\.venv\Scripts\activate.bat"

cmake -G Ninja %BUILD_OPTIONS% %PATHS% %CMAKE_PREFIX_PATH% %LANGUAGE% %BUILD_TYPE% %EDGETX_REPO%

REM 1. Збірка WebAssembly модуля симулятора (EdgeTX 3.0+)
ninja wasi-module

REM 2. Конфігурація та компіляція Companion і Simulator
ninja native-configure
ninja -C native simulator
ninja -C native simulatorConsole
ninja -C native companion

REM 3. Перенесення скомпільованого .wasm симулятора у папку Companion
if exist "wasm\wasm-build\*.wasm" (
    copy /y "wasm\wasm-build\*.wasm" "native\"
)

if "%BUILD_INSTALLER%"=="yes" (
    ninja -C native install
    %QT_DIR%\bin\windeployqt --libdir native\_install\bin --plugindir native\_install\plugins --no-compiler-runtime --no-translations --release native
    robocopy %QT_DIR%\bin\ native\_install\bin qt.conf
    "%NSIS_EXE%" native/companion/companion.nsi
)

%QT_DIR%\bin\windeployqt --libdir native --plugindir native\plugins --no-compiler-runtime --no-translations --release native
copy /y "%EDGETX_BUILD_TOOLS%\SDL2\lib\x64\SDL2.dll" "%EDGETX_MAIN_DIR%\%BUILD_FOLDER%\native\"

REM 4. Створення qt.conf для коректного пошуку плагінів інтерфейсу
(
    echo [Paths]
    echo Prefix = .
    echo Plugins = plugins
) > "native\qt.conf"

call "%EDGETX_BUILD_TOOLS%\.venv\Scripts\deactivate.bat"
pause