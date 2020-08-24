# LSL plugin for the EEG acquisition with ANTneuro device.

The released version for Windows and Linux can be found in the /release folder        [compiled date: 02.27.2020].

## Installation
### Windows
The Windows release has been compiled on Windows 10 with Qt Creator.

Missing files:
- win drivers
- eego-SDK.dll

If you work at Campus Biotech Geneva, contact the maintainer to receive the files otherwise contact directly the compagny to buy them.

To use the plugin, the driver has to be installed from the device manager. The Windows 8 driver can be used for Windows 10. 

### Linux
The Linux release has been compiled on Ubuntu 16.04 with Qt Creator. It will work on 16.04 and later.

Missing file:
- eego-SDK.lib

If you work at Campus Biotech Geneva, contact the maintainer to receive the file otherwise contact directly the compagny to buy it.

To avoid the need of superuser privilege, add the udev rule:
```
cd ./release/Linux
sudo cp 90-eego.rules /etc/udev/rules.d
sudo service udev restart
```

## Compilation from source code
A .pro file can be found to create the makefile using Qt Creator. The C++ boost lib is required.

## Usage
1. Select the ```CAP ID``` by looking directly on the cap you have. 
2. Select the desired ```sampling rate```
3. Pre-Selected ```EEG/BIP ranges``` are ok for most of the application.  
4. ```EEG/BIP masks``` allow to restrict the acquisition to a channels subpart. The mask is in hexadecimal. To prepare the mask, write from right to left a binary vector (e.g. 00101 to keep only Ch1 and Ch3) and then convert it to an hexadecimal (<https://www.rapidtables.com/convert/number/binary-to-hex.html>). Notice that all the zeros on the left after the last 1 will disappear during the conversion and need to be added again to the hex number.
5. Press ```Link``` button to start the data streaming to the LSL local server.

## Support
Contact <arnaud.desvachez@gmail.com>
