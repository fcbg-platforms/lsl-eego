# eego2lsl
LSL plugin for the EEG acquisition with ANTneuro device.

The released standalones for Windows and Linux can be find in the /release folder (02.27.2020).

## Windows
To use the plugin, the driver (in /eegoDriver) has to be installed. The Windows 8 driver can be used for Windows 10. Run .sys file and follow the instructions.

## Linux
The Linux standalone has been compiled on Ubuntu 14.04 and will work on 14.04 and later.

To avoid the need of superuser privilege, add the udev rule:
'''
cd ./release/Linux
sudo cp 90-eego.rules /etc/udev/rules.d
sudo service udev restart
'''
