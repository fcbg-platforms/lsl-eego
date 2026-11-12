# LSL plugin for the EEG acquisition with ANTneuro device.

A single code base builds for Windows and Linux. It streams one eego amplifier with a 64 channel cap (CA-208 or NA-265),
plus up to 24 bipolar/AUX channels.

## Download
Builds are produced by GitHub Actions ([.github/workflows/build.yml](.github/workflows/build.yml)):
- **Windows**: `eegoSports-windows-x64.zip` (contains `eegoSports.exe`, the Qt runtime, `lsl.dll` and `eego-SDK.dll`).
- **Linux**: `eegoSports-linux-x86_64.AppImage` (self-contained, includes `liblsl` and `libeego-SDK.so`).

Every push / pull request uploads them as workflow artifacts. Pushing a tag starting with `v` (e.g. `v2.0.0`) also attaches them to a GitHub release.

The eego SDK binaries are proprietary: they are stored encrypted in the repository (`*.enc`) and decrypted during the build with the `SDK_DECRYPT_PASSPHRASE` repository secret (see [scripts/encrypt-sdk.ps1](scripts/encrypt-sdk.ps1)). If you work at Campus Biotech Geneva, contact the maintainer; otherwise contact ANT Neuro directly to obtain the SDK.

## Installation
### Windows
Unzip the archive and run `eegoSports.exe`. The amplifier driver has to be installed from the device manager (the Windows 8 driver can be used for Windows 10/11).

### Linux
```
chmod +x eegoSports-linux-x86_64.AppImage
./eegoSports-linux-x86_64.AppImage
```

To avoid the need of superuser privilege, add the udev rule:
- Create the file ```90-eego.rules``` and copy paste:
```ATTRS{idVendor}=="2a56", ATTRS{idProduct}=="ee01", SYMLINK+="eego3.%n",MODE:="0666", ENV{DEVTYPE}=="usb_device"```
- Run:
```
sudo cp 90-eego.rules /etc/udev/rules.d
sudo service udev restart
```

## Compilation from source code
The project uses qmake ([eegoSports.pro](eegoSports.pro)) with Qt 6, the Boost headers (property_tree) and [liblsl](https://github.com/sccn/liblsl).
- Linux: install `libeego-SDK.so` and `liblsl.so` in the linker path, then `qmake6 eegoSports.pro && make`.
- Windows (MSVC): set `BOOST_ROOT` and `LSL_DIR`, put `eego-SDK.lib` in the build directory, then `qmake eegoSports.pro && nmake`.

The workflow file is the reference for the exact build steps.

## Usage
1. Select the ```CAP ID``` by looking directly on the cap you have. All the EEG channels of the cap are streamed:

   | Cap | EEG channels |
   |---|---|
   | CA-208 | 64 |
   | NA-265 | 64 |

   If several amplifiers are connected, the one with the lowest serial number is used.
2. Select the desired ```sampling rate```
3. Pre-Selected ```EEG/BIP ranges``` are ok for most of the application.
4. Tick the ```BIP channels``` (AUX1 to AUX24) to stream in addition to the EEG channels (```All``` / ```None``` buttons for convenience).
5. Press ```Link``` button to start the data streaming to the LSL local server.

The settings can be saved/loaded as a `.cfg` file (the BIP selection is stored as a hexadecimal mask, bit 0 = AUX1), and the application can be started from the command line with `-c <config file>` and `-l 1` to link automatically.

## Support
Contact <arnaud.desvachez@gmail.com>
