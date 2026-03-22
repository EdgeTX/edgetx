# Instructions for Installing FlySky FRM303 External Module to TX16S

The modification is quite simple, similar to Access MOD but no need the inverter.

Just need to reconnect the AUX1 output to PPM and Heartbeat pin directly. The following image shows how this can be done, I used connection plugs so that I can easily convert back to original circuit.

Please note that this mod will only affect the functionality of the external module, the internal module is not affected, so that one can use the internal 4 in 1 and external FRM303 together. After the mod, the ELRS external module should still works as well, as the ELRS ext module will only use the S.Port pin, but other modules that make use of the PPM pin may not work anymore after this mod.

![FRM303 wiring diagram](https://user-images.githubusercontent.com/18414861/230327856-c516c5e4-8fd8-41b4-b466-74912a5bfe6d.jpg)

For FRM303 setup, you need to first select the 1.5M opensource protocol, please refer to FRM303 manual for the procedures.

For radio settings, you need to setup AUX1 for External Module and then select Flysky in model settings, then everything done. Enjoy.

![FRM303 radio settings 1](https://user-images.githubusercontent.com/18414861/230498165-be66dcf3-adb1-4e14-a2a6-28d3630c5181.png)

![FRM303 radio settings 2](https://user-images.githubusercontent.com/18414861/230498177-42701d86-6335-4f65-9685-c36592fc1818.png)
