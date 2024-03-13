# Room Monitoring - NodeMCU+DHT

This Arduino sketch allows you to connect your device to a WiFi network and send data to a server. It fetches the server time and sends data to the anedya.

> [!WARNING]
> This code is for hobbyists for learning purposes. Not recommended for production use!!

## Set-Up

### [Dashboard Set-Up](https://docs.anedya.io/getting-started/quickstart/#create-a-new-project)

  1. Create account and login
  2. Create new project.
  3. Create variables: temperature and humidity.
  4. Create a node (e.g., for home- Room1 or study room).

 > [!TIP]
 > For more details, Visit [https://anedya.io/]

> [!IMPORTANT]
 > Variable Identifier is essential; fill it accurately.

### Hardware Set-Up

You can run the code without any hardware sensor also,Simply keep `virtual_sensor=true` or

To send hardware sensor value, keep `virtual_Sensor = false`

  1. Properly identify your sensor's pins.
  2. Connect sensor VCC pin to 3V3.
  3. Connect sensor GND pin to GND.
  4. Connect sensor signal pin to D4.

### Code Set-Up 

1. Replace `<PHYSICAL-DEVICE-UUID>` with your 128-bit UUID of the physical device.
2. Replace `<CONNECTION-KEY>` with your connection key, which you can obtain from the node description.
3. Set up your WiFi credentials by replacing `SSID` and `PASSWORD` with your WiFi network's SSID and password.
4. Specify the pin number connected to the DHT sensor.

## Usage

1. Connect your  device to a WiFi network.
2. Upload this code to your  device.
3. Open the Serial Monitor to view the device's output.
4. The device will connect to the WiFi network, read temperature and humidity data from the DHT sensor, and start sending data to the Anedya.

## Include library dependency

### ArduinoJson
This repository contains the ArduinoJson library, which provides efficient JSON parsing and generation for Arduino and other embedded systems. It allows you to easily serialize and deserialize JSON data, making it ideal for IoT projects, data logging, and more.

1. Open the Arduino IDE.
2. Go to `Sketch > Include Library > Manage Libraries...`.
3. In the Library Manager, search for "ArduinoJson".
4. Click on the ArduinoJson entry in the list.
5. Click the "Install" button to install the library.
6. Once installed, you can include the library in your Arduino sketches by adding `#include <ArduinoJson.h>` at the top of your sketch.


### Timelib
The `timelib.h` library provides functionality for handling time-related operations in Arduino sketches. It allows you to work with time and date, enabling you to synchronize events, schedule tasks, and perform time-based calculations.

To include the `timelib.h` library:

1. Open the Arduino IDE.
2. Go to `Sketch > Include Library > Manage Libraries...`.
3. In the Library Manager, search for "Time".
4. Click on the ArduinoJson entry in the list(`Time by Michael Margolis`).
5. Click the "Install" button to install the library.
6. Once installed, you can include the library in your Arduino sketches by adding `#include <TimeLib.h>` at the top of your sketch.


### DHT-Adafruit
The DHT library provides support for DHT sensors (DHT11, DHT21, DHT22, AM2301, AM2302, AM2321) on ESP32 boards. It enables easy interfacing with DHT sensors to read temperature and humidity data accurately.

To include the DHT library in your project:

1. Install the DHT library through the Arduino IDE Library Manager. | URL https://github.com/adafruit/DHT-sensor-library
2. Go to `Sketch > Include Library > Manage Libraries...`.
3. In the Library Manager, search for "DHT" .
4. Click on the DHT entry in the list (DHT sensor Library by Adafruit Adafruit).
5. Click the "Install" button to install the library.
6. Once installed, you can include the library in your Arduino sketches by adding `#include <DHT.h>` at the top of your sketch.

> [!TIP]
> For python SDK, visit  [https://github.com/anedyaio/anedya-dev-sdk-pyhton]

>[!TIP]
> Visit [Anedya.io](https://anedya.io/)
 