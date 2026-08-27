# Workflow for GitHub Noobs

This guide is targeted for Ubuntu Linux users, who want to help testing EdgeTX firmware and or LUA scripts, but who have not yet found their way around Git and GitHub.

# How does GitHub work

EdgeTX project uses GitHub services e.g. for distributed version control and source code management functionalities. As an open-source project, the source code can be downloaded by everyone. Users can modify and augment the code and propose the changes as so called pull requests. But this page is not about the latter. Here, we will concentrate on getting EdgeTX code from GitHub, in order to compile and test it locally.
If you used the guide to get Ubuntu running on a Windows subsystem, or having another Ubuntu environment, and you followed the instructions to get the compile toolchain installed, then you have already installed the GIT software with it, so no actions there.

# The workflow

## Getting the code

Well, if you followed the installations and first instructions to the letter, you already did the following things:

* Created a folder for managing code in your personal /home/user directory (~)

```
mkdir ~/edgetx
cd ~/edgetx
```

* "Cloned" the code from the public repository

```
git clone --recursive -b main https://github.com/EdgeTX/edgetx.git edgetx-main
```

 * Created a subfolder for the _main_ branch by doing the above (~/edgetx/edgetx-main)

That's good!

## Compiling

I will list all the steps to compile the code into the firmware for on the radio, the full simulator, and companion. The goal is that we test things first in the simulator, and then on the physical radio. Saves a lot of hassle.

### Preparation

```
cd ~/edgetx/edgetx-main
```

If it doesn't exist yet from a previous build session, create a build output folder:

```
mkdir build-output
```

Move to the build-output folder and start preparation of the CMAKE file (the file that contains the instructions to compile all stuff):

```
cd build-output
```

Don't forget to add the path for this session:

```
export PATH="~/edgetx/edgetx-main/radio/util:$PATH"
```

(You can add it to your environment permanently. Search google).
And then create the makefile in the right spot:

```
cd ~/edgetx/edgetx-main/build-output
cmake -DPCB=X10 -DPCBREV=TX16S -DDEFAULT_MODE=2 -DGVARS=YES -DPPM_UNIT=US -DHELI=NO -DLUA=YES -DINTERNAL_GPS=YES -DCMAKE_BUILD_TYPE=Debug ../
```

Change the above to your preferences of course :). See _~/edgetx-main-cmake-options.txt_.

### Compiling the firmware

```
make -j`nproc` firmware
```

The firmware created to load on your radio is _firmware.bin_.

### Compiling Companion

```
cd ~/edgetx/edgetx-main/build-output
make -j`nproc` companion25
```

To launch Companion, issue: `./companion25`
Do that first, to create at least ONE profile to be able to use the simulator.

### Compiling the simulator

Remember it will use the settings as prepared in the cmake command.

```
cd ~/edgetx/edgetx-main/build-output
make -j`nproc` libsimulator && make -j`nproc` simulator25
```

### Compiling all in one go

Set the path if you didn't do it yet

```
export PATH="~/edgetx/edgetx-main/radio/util:$PATH"
```

Do the rest

```
cd ~/edgetx/edgetx-main/build-output
cmake -DPCB=X10 -DPCBREV=TX16S -DDEFAULT_MODE=2 -DGVARS=YES -DPPM_UNIT=US -DHELI=NO -DLUA=YES -DINTERNAL_GPS=YES -DCMAKE_BUILD_TYPE=Debug ../
make -j`nproc` firmware
make -j`nproc` companion25
make -j`nproc` libsimulator
make -j`nproc` simulator25

```

## Running companion or the simulator

### Preparation

You will need to have some location for the SDCARD you are going to use for the simulation. I suggest to set up an SDCARD folder in your personal directory, and create a subfolder for all radio types. Yes, if you are going to test, it is good practice to not only test your own preferred radio, but to test at least one of each of the other versions. It is a courtesy to the developer and future users:

```
mkdir ~/SDCARD
cd ~/SDCARD

wget https://github.com/EdgeTX/edgetx-sdcard/releases/download/latest/taranis-x7.zip
wget https://github.com/EdgeTX/edgetx-sdcard/releases/download/latest/taranis-x9.zip
wget https://github.com/EdgeTX/edgetx-sdcard/releases/download/latest/horus.zip
unzip -u taranis-x7.zip
unzip -u taranis-x9.zip
unzip -u horus.zip
rm *.zip

```

### Running companion

The companion is located in the build_output directory, so we run it with this command:

```
~/edgetx/edgetx-main/build-output/companion25
```

Now when companion started, you need to go to settings/radio profiles and create at least one profile, with the created radio type selected. Also, in settings, set the SDcard path to your just downloaded and unzipped SDCard content, in this tutorial the Horus content.

````
/home/yourUserName/SDCARD/horus
````

If you have an existing .otx file, you can now load is at well :).

### Running the simulator

The simulator is located in the build_output directory as well, so we run it with this command:

````
~/edgetx/edgetx-main/build-output/simulator25
````

Now when companion started, you created at least one profile, with the created radio type.
Select that profile, and start it up.

## Doing testwork

Now you have a running simulator, you can test your own models by opening your .otx file in the simulator. Be aware that the simulator will actually CHANGE that file, so better use copies only. If the simulator works correct, you can try using the firmware on the radio.
You can just copy the firmware you created to the SDCard of your radio, in the FIRMWARE folder. Then, when starting the radio in BOOTLOADER mode by pushing the horizontal trims to each other while starting the radio, you can select the new firmware, and install it.
Make sure you have backups if needed, and a correct firmware somewhere to be able to go back when necessary.

### Reporting an issue

Well, if you found any issue, please first check if it is not an existing issue on github.
[EdgeTX open issues](https://github.com/EdgeTX/edgetx/issues?q=is%3Aissue+is%3Aopen)

You can then add your findings to the existing issues, making it easier to solve it (correctly).
Otherwise, create a new issue.
[new issue](https://github.com/EdgeTX/edgetx/issues/new/choose)
At least, mention the radio type and firmware version in your issue, as well as if it was the simulator or the physical radio that showed the bug, or both.

### Updating your copy of the repo after a fix was pushed

This is the part where all the work was done for. By having the possibility to compile yourself, you don't need to wait for the release candidates to help testing certain changes!
You can now update the code you cloned before at anytime to the latest version by the following command:

````
cd ~/edgetx/edgetx_main
git pull https://github.com/EdgeTX/edgetx.git
````

This video gives some useful tips:
[github tutorial](https://www.youtube.com/watch?v=HVsySz-h9r4)

## Some tips for Windows adepts

* You can start an explorer from Ubuntu, that will allow you to access the Ubuntu files safely from Windows. Simple type `explorer.exe` in Ubuntu. In the explorer you open from Ubuntu in Windows:
* The Ubuntu files are under `\\wsl$\Ubuntu-20.04\`
Pin it to you quick access list of locations!! there is a small pin in the start menu on explorer to do that.

Make sure you know where you are in Ubuntu Linux:
* the terminal starts in your personal directory in Ubuntu, just one level below `/HOME`. So type `cd /HOME`, and then `dir`, and you can see where your personal files are for your Ubuntu user.
* The X-windows terminal starts in `/mnt/c/Windows/System32`. To switch quickly to your home directory type `cd /HOME`

## Conclusion

It is not that difficult when you got it al listed out to setup and compile your own stuff, test it, report it back, and redo the entire cycle.

I hope others feel the same when using this guide.

Next guide will be on documentation.

# Open Point

I am really a noob. That helps to prevent the obvious things from being forgotten to write down for other noobs. Like setting the path on every session before the compile, and other stupid simple things you do without thinking when you do things more often.

In this workflow, we are NOT interested in keeping things up to date, correct, organized, from a coding perspective.

It is meant to reload another branch or tag or version or whatever things there are on GitHub that need testing or documentation (new parts in the GUI for instance) with as little as possible actions. And without ruining any settings or references that are already created for the companion and or simulator that are probably stored when running in the build-output directory.

What I am not understanding yet is how to test a certain pull request before it is part of the main branch.

If someone could help out there, that would be great.
