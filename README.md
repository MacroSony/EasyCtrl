# EasyCtrl

A mbed program for discovery board DISCO-L475VG-IOT01A that utilize it's VL53L0X TOF sensor and the wifi module ISM43362-M3G-L44 to perform gesture based LGTV control. 

**Note:** The project's client script is for Windows PC and LG webOS TV only.


## Usage Guide

### Initial Setup:

1. Project Setup:
   - Install the following libs:
        DISCO_L475VG_IOT01A_wifi: http://os.mbed.com/teams/ST/code/DISCO_L475VG_IOT01A_wifi/
        VL53L0X: https://os.mbed.com/teams/ST/code/VL53L0X/
   - Change the "wait_ms(\<ms\>)" function calls in VL53L0X lib's VL53L0X.h to wait_us(\<ms\> * 1000)
   - Install https://github.com/JPersson77/LGTVCompanion and follow it's instruction to setup, the client script will use LGTVcli.exe to send command to the TV
   - Install Python3, the script is tested on python 3.10, but the version shouldn't matter too much.


2. Program Setup:
   - Input the Wi-Fi SSID and password into the `mbed_app.json` file for connection.
   - Build and run the program.
   - Connect the discovery board to a serial monitor with baud rate 115200, such as the serial monitor output section in Mbed Studio on your PC. You will see the board connecting to Wi-Fi and printing a MAC address.
   - Copy and paste this MAC address into the corresponding location in the client program's settings and save it to complete the basic setup.
   - Copy the path of "LGTVcli.exe" to the client script

### Running the Program:

1. Program Execution:
   - Press the reset button to run the program on the board again.
   (at this time you can connect the board to any powered source, such as a power bank or another computer).
   - run the client program
   - Perform gestures above the board to control your LG TV:
     - Tap once to turn on/off the screen.
     - Swipe once to adjust screen brightness.
     - Swipe twice to switch HDMI input sources.

**Note:** Before running the program, download, install, and set up the LGTVCompanion project on your device.
https://github.com/JPersson77/LGTVCompanion Modify the path in the client program if necessary, based on your installation location.
