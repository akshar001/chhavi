# Chhavi - A Key to Unlock Everything! Setup Guide

## Welcome to the Chhavi Ecosystem!

This guide will help you get started with your ESP32-based NFC and fingerprint device. Whether you're a developer, tinkerer, or just curious about this technology, we've got you covered.

## Table of Contents

1. **Introduction**
2. **Prerequisites**
3. **Setting Up**
    - Installing Required Libraries
    - Uploading the Example Sketch
4. **Serial Communication**
5. **Additional Resources**
6. **Contact and Support**

## 1. Introduction

Chhavi is an open-source project that offers an ESP32-based NFC and fingerprint device. This device has a range of applications, from security systems to access control. Our goal is to provide a seamless and accessible platform for developers and enthusiasts to explore and utilize NFC and fingerprint technologies.

## 2. Prerequisites

Before you dive into the Chhavi project, make sure you have the following:

- **Chhavi Device:** Ensure you have received the Chhavi device, either through distribution or assembly.
- **Computer:** You'll need a computer to program the device and interact with it.
- **Micro-USB Cable:** Use this to connect the Chhavi device to your computer.
- **Arduino IDE:** Install the Arduino Integrated Development Environment (IDE) from [arduino.cc](https://www.arduino.cc/en/software).
- **CP2102 Driver:** Install the driver to establish serial communication with Chhavi. Download the driver from [Silicon Labs](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads) and choose the Virtual COM Port (VCP) driver.

## 3. Setting Up

### Installing Required Libraries

To start with Chhavi, install these Arduino libraries:

- [Ticker](https://github.com/sstaub/Ticker)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306) and its dependent libraries.

Install libraries via Arduino Library Manager:

1. Open Arduino IDE.
2. Go to **Sketch > Include Library > Manage Libraries**.
3. Search for each library and click **Install**.

### Running the Example Project

To begin exploring the capabilities of your Chhavi device, we've provided an example project located in the `Firmware -> Arduino -> Example_Websocket` folder. This example project hosts an HTTP server and WebSocket server, allowing fingerprint sensors to communicate with the device and display live data in a web browser.

Follow these steps to run the example project:

1. Open Arduino IDE.
2. Go to **File > Open** and navigate to the Chhavi repository.
3. Inside the repository, navigate to `Firmware -> Arduino -> Example_Websocket` and open the `Example_Websocket.ino` file.
4. In the Arduino IDE, ensure you have selected the correct board. Go to **Tools > Board** and choose the **ESP32 WROOM** module option.
5. Update the WiFi credentials to match your network. Look for the following line in the code (usually around line 14):

   ```cpp
   const char* ssid = "your_wifi_ssid";
   const char* password = "your_wifi_password";


## 4. Serial Communication

To communicate with Chhavi via serial:

1. Connect Chhavi to your computer.
2. Open Arduino IDE.
3. Go to **Tools > Serial Monitor**.
4. In Serial Monitor, select the baud rate (usually 115200) from the dropdown at the bottom.
5. Observe the device's serial output for debugging and monitoring.

## 5. Additional Resources

- For hardware resources, such as design files and schematics, refer to our crowd-supply campaign page once active.
- For firmware resources, explore the code in our repository.

## 6. Contact and Support

For questions, issues, or sharing your Chhavi projects, contact us at [contact@vicharak.in](mailto:contact@vicharak.in).

Thank you for joining us on this exciting Chhavi journey!

Remember, Chhavi is not just a device; it's a key to unlock endless possibilities.

Happy tinkering!

Discord server!

https://discord.gg/BWkbF5Sg
