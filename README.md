# EasyNetVibe

EasyNetVibe is an M5StickC-based remote controllable vibrator. This device can be controlled by Buttplug applications.

<img src="./images/photo.png" alt="Photo Image" width="200">

## Features

BLE Connectivity: Allows wireless control using BLE, supporting the Lovense device protocol.
Haptic Feedback: Integrated with a haptic driver to provide vibrational feedback.
Battery Monitoring: Displays and transmits battery status via BLE.
Customizable Display: Includes an on-device visual display for battery level and vibration status.
Easy Control: Simple controls for power and vibration settings via a mobile or desktop application.

## Hardware Requirements

- [M5StickC PLUS2 ESP32 Mini IoT Development Kit](https://shop.m5stack.com/products/m5stickc-plus2-esp32-mini-iot-development-kit)
- [SparkFun Qwiic Haptic Driver - DA7280](https://www.sparkfun.com/products/17590)
- [Qwiic Cable - Grove Adapter (100mm)](https://www.sparkfun.com/products/15109)


## Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software) with [M5Stack library](https://github.com/m5stack/M5StickC)
- Buttplug compatible application for sending BLE commands
    - [Intiface Central](https://intiface.com/central/)
    - [ButtplugLite](https://github.com/runtime-shady-backroom/buttplug-lite)
    - [etc.](https://github.com/buttplugio/awesome-buttplug?tab=readme-ov-file)


## Installation

1. Clone the repository. Please refer [this document](https://docs.github.com/ja/repositories/creating-and-managing-repositories/cloning-a-repository).
1. Install Arduino IDE. Please refer [this document](https://support.arduino.cc/hc/en-us/articles/360019833020-Download-and-install-Arduino-IDE)
1. Opne EasyNetVibe.ino sketch by Arduino IDE
1. [Install M5Stack library](https://docs.m5stack.com/en/arduino/arduino_library). 
1. Install SparkFun Qwiic Haptic Driver DA7280 Arduino Library. Please refer to the [manufacturer's documentation](https://learn.sparkfun.com/tutorials/qwiic-haptic-driver-da7280-hookup-guide).
1. Connect SparkFun Qwiic Haptic Driver to M5StickC2 Plus by Qwiic Cable - Grove Adapter.
1. Connect M5StickC2 Plus to your PC/Mac by USB-C cable.
1. Check the Serial Poprt to M5StickC2. "If you're using Windows, you should be able to find it in the Device Manager under the COM ports.
1. [Select the serial port in Arduino IDE.](https://support.arduino.cc/hc/en-us/articles/4406856349970-Select-board-and-port-in-Arduino-IDE)
1. Select board type as `M5Stick-C-Plus`.
1. Upload the sketch to M5StickC2.
1. Open and run your Buttplug application. In the case of Intiface Central, after running the Server, you can start scanning for devices by selecting `Start scanning` from the `Devices` menu, and this device will be detected as Lovense Hush.

# Note

- The M5StickC Plus2 does not measure battery voltage correctly while using the radio function. Therefore, the battery level used for the display is the value measured before the BLE connection.

- Because M5StickC Plus2 measures battery voltage with ADC, the battery level value does not correctly represent the SoC. Therefore, it shuts down due to insufficient remaining power even if the battery remains at around 70%.

# License
This project is licensed under the MIT License - see the [LICENSE](./LICENSE) file for details.

# Author

[Herbst17904634](https://x.com/Herbst17904634)