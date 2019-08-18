# regler

## Prerequisites

1. Install xtensa toolchain:

    https://docs.espressif.com/projects/esp-idf/en/latest/get-started/linux-setup.html

1.  Install esp-idf:

    https://docs.espressif.com/projects/esp-idf/en/stable/get-started/

1. Install Arduino component:

    https://github.com/espressif/arduino-esp32/blob/master/docs/esp-idf_component.md

## Tested components:
* ESP-IDF: espressif/esp-idf@106dc05903a1691c024bb61ac7b29ca728829671
* Arduino: espressif/arduino-esp32@7df50a97d17b0953ea01cad0355410a66bd8b8b4

## Configuration/Build/Flash
Unix
```console
# Init (once for fresh build)
make defconfig
# Configuration
make menuconfig
# Build
make
# Monotir
#   Flash All:  Ctrl+T Ctrl+F
#   Flash App:  Ctrl+T Ctrl+A
#   Reset:      Ctrl+T Ctrl+R
make monitor
# Flash
make flash
```
Windows
cd %userprofile%\esp\regler-master
idf.py build

## GPIO configuration
GPIO num | Description | Type
------------ | ------------- | -------------
GPIO25 | LED transmitter 1 | Digital Output
GPIO27 | LED transmitter 2 | Digital Output
GPIO32 | LED receiver 1 | Digital Input
GPIO35 | LED receiver 2 | Digital Input
GPIO26 | Barrier speaker | Analog Output

## Manual Flash
python /home/USER/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port COM4 --baud 115200 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 40m --flash_size detect 0xe000 /home/USER/esp/Regler-dev2/build/ota_data_initial.bin 0x1000 /home/USER/esp/Regler-dev2/build/bootloader/bootloader.bin 0x10000 /home/USER/esp/Regler-dev2/build/regler.bin 0x8000 /home/USER/esp/Regler-dev2/build/default.bin



