TJFTL
=====

This is a Tiny, Journalling Flash Translation Layer. It is meant for NOR devices, mostly targeted
at the 25xxx SPI flash memories and alike (256 byte page size, 4/32/64K erase size, around 16MiB
of storage.)


What does this do?
==================

One of the easiest USB protocols to implement to give a computer access to the storage on your device
is Mass-Storage. This, however, implies that you need to emulate a block device with 512b sectors. Also,
the most compatibe filesystem that OSses are going to understand is FAT, which does not have inherent
protection from corruption. FAT is also intended for 'spinning rust'-style disks that do not need wear
leveling like flash does.

This translation layer is supposed to be a 'go-between'; on one hand, it puts data on the flash in a
way where it is aware that things like erase sizes make it hard to write 512b sectors, and where it
knows it shouldn't erase the same sector too often. On the other hand, to the upper layers, it
shows an interface that very much makes it looks like a hard disk with 512b sectors.
