## Usage Guide

### Initial Setup:

1. First-time Setup:
   - Input the Wi-Fi SSID and password into the `mbed_app.json` file for connection.
   - Build and run the program.
   - Connect the discovery board to a serial monitor, such as the serial monitor output section in Mbed Studio on your PC. You will see the board connecting to Wi-Fi and printing a MAC address.
   - Copy and paste this MAC address into the corresponding location in the client program's settings and save it to complete the basic setup.

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
