# Build Instructions using Docker and Windows 10

Work-in-progress page

# Building EdgeTX on Windows 10 using Docker images

## Download and install [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop/)

## Download and install [GWSL](https://github.com/Opticos/GWSL-Source/releases/) to run Linux Companion and Simulator

- Run GWSL to configure it. Accept page 1 and 2 defaults and add disable access control on page 3


## Fetch a local copy of the EdgeTX repository or update your existing local EdgeTX repository
```
git clone --recursive -b main https://github.com/EdgeTX/edgetx.git EdgeTX
```
- If you already have a local EdgeTX repository don't forget to update the submodules too, e.g.:
```
git fetch origin
git checkout main
git submodule update --init --recursive
```

## Run build (example TX16s)

- open a PowerShell
- cd to your local repository
```
docker run --name="ETXDocker" -it --rm --mount src="$(pwd)",target="/src",type=bind ghcr.io/edgetx/edgetx-dev bash
cd src
mkdir -p build-output
cd build-output
cmake -DPCB=X10 -DPCBREV=TX16S -DCMAKE_BUILD_TYPE=Release ../
make configure
make -C arm-none-eabi -j$(nproc) firmware
make -C native -j$(nproc) libsimulator
make -C native -j$(nproc) companion210
make -C native -j$(nproc) simulator210
```
- find the results in folder build-output of your local build repository folder

# More Cmake build options

X9d+:
```
-DPCB=X9D+ -DCMAKE_BUILD_TYPE=Release
```

X7:
```
-DPCB=X7 -DCMAKE_BUILD_TYPE=Release
```
TX12:
```
-DPCB=X7 -DPCBREV=TX12 -DCMAKE_BUILD_TYPE=Release
```

TX16S:
```
-DPCB=X10 -DPCBREV=TX16S -DCMAKE_BUILD_TYPE=Release
```

# Running Linux Companion and Simulator

## Running Companion

- make sure GWSL is running
- open a PowerShell
- in case you don't know your IP address enter 'ipconfig' to find out
- cd to your build folder
- remember to replace IP_OF_YOUR_WINDOWS_MACHINE with your IP address and enter:
```
docker run --name="ETXDocker" -it --rm --mount src="$(pwd)",target="/src",type=bind ghcr.io/edgetx/edgetx-dev bash
export DISPLAY=IP_OF_YOUR_WINDOWS_MACHINE:0.0
cd src
./native/companion210
```

## Running Simulator

- make sure GWSL is running
- open a PowerShell
- in case you don't know your IP address enter `ipconfig` to find out
- `cd` to your build folder
- remember to replace IP_OF_YOUR_WINDOWS_MACHINE with your IP address and enter:
```
docker run --name="ETXDocker" -it --rm --mount src="$(pwd)",target="/src",type=bind ghcr.io/edgetx/edgetx-dev bash
export DISPLAY=IP_OF_YOUR_WINDOWS_MACHINE:0.0
cd src
./native/simulator210
```

# Needful things

- Should you get a message like ```container name "/ETXDocker" is already in use``` open a PowerShell and enter ```docker stop ETXDocker```
- If you need to update the docker image open a PowerShell and enter: ``` docker rmi ghcr.io/edgetx/edgetx-dev```. The latest image will then be downloaded at next docker run
- before running the simulator you need to run companion and create a radio profile
- companion and simulator won't have audio. If you need audio you might find [PulseAudio on WSL2](https://www.linuxuprising.com/2021/03/how-to-get-sound-pulseaudio-to-work-on.html) useful

# References

see [EdgeTX Build Wiki](https://github.com/EdgeTX/build-edgetx)
