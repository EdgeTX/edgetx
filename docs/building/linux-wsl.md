# Setting up Ubuntu 20.04 in a Windows Subsystem for Linux

# For who is this guide?

This guide is especially useful for people who want to help testing firmware and or LUA scripts. It will allow to run a simple simulator with the LUA debug and all other radio functions on your W10 64-bit computer in a separate window, running Ubuntu.

# How does this work

Windows 10 / Windows 11 is now able to run another OS (Linux) in a subsystem. However, this subsystem does not have access to the screen as it is shipped. By default, at the time of writing (as it will change in the near future with WSLg) it only offers a terminal interface. By using an X window server (in this guide we use VcXsrv) a window can be created to which the subsystem can connect to output graphical content. This can be used to be used to run the radio simulator in. We will install Ubuntu on the subsystem, so we are ready to use all the build tools already supported.

# Installation procedure

We are going to install the following things:

* WSL2 (Windows Subsystem for Linux, version 2)
* A Linux distribution (Ubuntu 20.04 LTS)
* Setup the EdgeTX build environment in order to be able to compile firmware and simulators
* Setup the X-Windows server (VcXserver) so that we can run graphical programs like the simulator

## WSL2 installation

You can use the [guide from Microsoft](https://docs.microsoft.com/en-us/windows/wsl/install-win10). It is really straightforward. The steps for the manual install are below:

* In the search box in the Windows taskbar (or press Window(KEY)+R) type "powershell". It will list all possible way of running Windows PowerShell. Run it as an administrator.
* In the PowerShell window, type `wsl --install`
* Restart your computer

## Ubuntu installation

Ubuntu (20.04) is installed automatically.

* After the reboot a Ubuntu WSL window will open and after a short while you will be asked to create a Username and Password for the OS being installed. Choose something yourself you can remember ;)
* It is good practice to first load any updates, so type `sudo apt-get update` and press enter to check for updates, and `sudo apt-get upgrade` to install all the available updates. The `sudo` in this command tells the OS to do an update while being logged in as SuperUser (SUDO stands for SuperUser Do). It will ask for the password you used when you installed the OS. Without it you can't run these sorts of commands, so don't forget it!

## Settings up the build system

Now that you have an up to date Ubuntu 20.04 Linux WSL2 ready, it's time to configure it for EdgeTX. Most of the hard work will be done for you via a per-prepared script. But before you run that, you have to enable support for the i386 libraries in order to be able to compile, and then you can download and run the installation helper script.

* To enable the needed i386 support, run
```
sudo dpkg --add-architecture i386
```
* To download the installer script, run
```
wget https://raw.githubusercontent.com/EdgeTX/edgetx/main/tools/setup_buildenv_ubuntu20.04.sh
```
* in order to make the script executable (so you can run it), run
```
chmod a+x setup_buildenv_ubuntu20.04.sh
```
* To finally execute/run the installer script, run
```
./setup_buildenv_ubuntu20.04.sh
```

If all went smoothly, you should not have seen any errors.

Although the script suggests you need to reboot the computer, this is not generally the case, and is partly defeats the purpose of WSL2. All that is needed is for you to run
```
source ~/.bashrc
```
in order for some settings to the path to be applied.

## For Windows 11 you're now done and you can switch to the Ubuntu Guide

- [Ubuntu 20.04 Build Instructions](linux-ubuntu-22.04.md)

## Preparing for graphical applications

* Now we will immediately install the terminator we need to connect to the X-window later: `sudo apt-get install terminator` and press enter.
Accept all confirmation requests and enter your password when needed.
* As preparation for later we will install a graphical editor you may like: `sudo apt install gedit` and press enter.
You can try to run it, but since we have no graphical output yet, that wont work. We will use it later.

If you are not familiar with Ubuntu or other Linux systems, things might be scary. Don't worry, we are not going to do much with it. just type "exit" and press enter to close down Ubuntu properly and close the terminal window. Use the shortcut if windows created it to start it again, or search for "Ubuntu" using the loupe again.

### VcXserver installation

This will enable us to run an application with a graphical output, such as the simulator, or companion, on our fresh Ubuntu install.

* First you need to download it [from here](https://sourceforge.net/projects/vcxsrv/) and install it as administrator.
* Then we will create a shortcut on the desktop so we can start it like we want it. Rightclick anywhere on the screen, select new, shortcut and add the following text as destination: `"C:\Program Files\VcXsrv\vcxsrv.exe" :0 -ac -terminate -lesspointer -multiwindow -clipboard -wgl -dpi auto`
* Finally, we need a VBS script that we can start from a second shortcut to launch Ubuntu in the X-window.
* Go to the public documents folder `C:\Users\Public\Documents` or any other place you can remember.
Create a new text document (rightclick anywhere on the desktop, select new, textfile) and name it "startTerminator.txt". Open it and enter the following text: `args = "-c" & " -l " & """DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0 terminator"""
WScript.CreateObject("Shell.Application").ShellExecute "bash", args, "", "open", 0`
* Rename it to "startTerminator.vbs".
* Go to the desktop and create a shortcut (rightclick on the screen, select new, shortcut) and enter the following text: `C:\Windows\System32\wscript.exe C:\Users\Public\Documents\startTerminator.vbs`
This finishes the install.

### How to start using it

* Now to test all this, I suggest to disable the firewall for a moment. Use the windows search loupe or press Window(KEY)+R and type 'firewall', open it, and disable it for all three levels. If you want, you can shut down your wifi first for security reasons.
* now, first open the Xserver by clicking on the shortcut we made for it
* then open the Ubuntu session and link it to the Xserver by clicking on the second shortcut.
* Make sure you start both as ADMINISTRATOR
* You should now, after a short delay, get an Ubuntu terminal window, but now running within the Xserver window.
* we can now start using our graphical text editor we installed earlier. type `gedit` and enter.
* after all this is working, you can start the firewall again, and start tweaking it to allow the WSL to communicate properly with the firewall active.
* At least you should allow VcXserver windows Xserver as app to have acces to both private and public networks.

## Running the SIMU

We have created a SIMU application for the Taranis X9D+ by compiling it for you. If you want to test it, just type the following command to fetch it `wget -q https://raw.githubusercontent.com/wiki/EdgeTX/edgetx/files/simu/simu_x9dplus_ubuntu`.
then `chmod a+x simu_x9dplus_ubuntu` and run it with `./simu_x9dplus_ubuntu`.
And your X9D+ simple simulator is running in an Ubuntu window on your windows computer. (the key shortcuts can be found in the windows companion help screen: PG-UP->MENU; PG-DN->PAGE; DEL/ESC/BKSP->EXIT; +->+; -->-; ENTER->ENT).

## Compiling

In order to make the instructions that are on another WIKI page for setting up UBUNTU to compile, there is only one thing you need to do first.

* You have to enable the support for the i386 libraries by executing the following commands in UBUNTU:
* `sudo dpkg --add-architecture i386`
* `sudo apt-get update`

Now you can just follow the standard UBUNTU guide, and off you go!

- [Ubuntu 20.04 Build Instructions](linux-ubuntu-22.04.md)

## Some tips for Windows adepts

* You can start an explorer from ubuntu, that will allow you to access the UBUNTU files SAFELY from windows. Simple type `explorer.exe` in UBUNTU
In the explorer you open from UBUNTU in windows:
* The UBUNTU files are under `\\wsl$\Ubuntu-20.04\`

Make sure you know where you are in UBUNTU:

* the UBUNTU terminal starts in your personal directory in UBUNTU, just one level below `/HOME`. So type `cd/HOME`, and then `dir`, and you can see where your personal files are for your UBUNTU user.
* The X-windows terminal starts in `/mnt/c/Windows/System32`. To switch quickly to your home directory type `cd /HOME`

## Conclusion

You can thus set up companion on UBUNTU to use your windows directories for the SDcard and the ETX files, working seamless together with windows from an UBUNTU X-window. The same goes for the simulator.

In essence, from a functional perspective, Companion24 and the full Simulator24 will be exact the same as if they where compiled and run under windows directly.
