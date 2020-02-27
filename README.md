# LSL plugin for the EEG acquisition with ANTneuro device.

The released standalones for Windows and Linux can be found in the /release folder        [compiled date: 02.27.2020].

## Installation
### Windows
The Windows standalone has been compiled on Windows 10 with Qt Creator.

To use the plugin, the driver has to be installed. The Windows 8 driver can be used for Windows 10. 
Run the file ```cyusb3.sys``` and follow the instructions.

### Linux
The Linux standalone has been compiled on Ubuntu 14.04 with Qt Creator. It will work on 14.04 and later.

To avoid the need of superuser privilege, add the udev rule:
```
cd ./release/Linux
sudo cp 90-eego.rules /etc/udev/rules.d
sudo service udev restart
```

## Usage
1. Select the CAP ID by looking directly on the cap you have. 
If unusual montages, choose No labels options; it will display Ch1,..., ChX in the data.
2. Select the desired sampling rate
3. Pre-Selected EEG/BIP ranges are ok for most of the application.  
4. EEG/BIP masks allow to restrict the acquisition to a channels subpart. The mask is in hexadecimal. To prepare the mask, write from right to left a binary vector (e.g. 00101 to keep only Ch1 and Ch3) and then convert it to an hexadecimal (<https://www.rapidtables.com/convert/number/binary-to-hex.html>). Notice that all the zeros on the left after the last 1 will disappear during the conversion and need to be added again to the hex number.
5. Link to start the data streaming to the LSL local server.

## Support
Contact <arnaud.desvachez@gmail.com>
