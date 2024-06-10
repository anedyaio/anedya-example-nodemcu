[<img src="https://img.shields.io/badge/Anedya-Documentation-blue?style=for-the-badge">](https://docs.anedya.io?utm_source=github&utm_medium=link&utm_campaign=github-examples&utm_content=nodeMcu)

# Basic-Home-Automation

<p align="center">
    <img src="https://cdn.anedya.io/anedya_black_banner.png" alt="Logo">
</p>

This project is designed for users looking to implement a basic home automation system. The Arduino sketch provided enables users to monitor key environmental parameters such as humidity and temperature, which are crucial for maintaining a comfortable living space. Additionally, the system allows for remote control of connected devices, 
offering convenience and enhancing the overall smart home experience.

With Anedya's platform, you can send commands to your devices and receive real-time updates on your environment's conditions. The integration with the [anedya-streamlit-dashboard-example](https://github.com/anedyaio/anedya-streamlit-dashboard-example) web app provides a user-friendly and customizable interface for interacting with your home automation system. The Anedya dashboard further enhances this by offering advanced features and analytics to help you optimize your home automation setup.

## Getting Started

To get started with the examples:

1. Choose the code, create the .ino file, and open it in the Arduino IDE.
2. Fill in your Wi-Fi credentials, physical device ID, and connection key (obtained from the dashboard).
3. Select the Board and Port number.
4. Upload the code.

## Usage
 
### anedya-streamlit-dashboard-example setup [Repo](https://github.com/anedyaio/anedya-streamlit-dashboard-example.git)

![Image](https://github.com/anedyaio/anedya-streamlit-dashboard-example/blob/main/docs/anedya_dashboard.png)

1. Clone the Repository:
Clone [aneday-streamlit-dashboard-example](https://github.com/anedyaio/anedya-streamlit-dashboard-example.git) repo from anedya github.
```
git clone https://github.com/anedyaio/anedya-streamlit-dashboard-example.git
cd anedya-streamlit-dashboard-example

```
2. Install Dependencies:
To ensure the application has all the necessary libraries, you'll need to install the dependencies listed in the requirements.txt file. This file contains a list of all the packages along with their versions that the app depends on.
Here's how to install these dependencies:
- Open a terminal or command prompt.
- Navigate to the directory where you cloned the repository.
- Run the following command to install the dependencies using pip:
```
pip install -r requirements.txt
```
This command reads the `requirements.txt` file and installs all the listed packages.

3. Obtain your nodeid and apikey from the Anedya dashboard. These credentials are necessary for connecting your application to the Anedya platform. Once you have these, You need to replace <PHYSICAL-DEVICE-UUID> and <CONNECTION-KEY> with your actual device ID and connection key, respectively. 

4. Run the Streamlit App:
Start the Streamlit server:
```
streamlit run Home.py

```
This will launch the dashboard in your default web browser.

## Dependencies

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

1. Install the DHT library through the Arduino IDE Library Manager. You can get the library from [here](https://github.com/adafruit/DHT-sensor-library)
2. Go to `Sketch > Include Library > Manage Libraries...`.
3. In the Library Manager, search for "DHT" .
4. Click on the DHT entry in the list (DHT sensor Library by Adafruit Adafruit).
5. Click the "Install" button to install the library.
6. Once installed, you can include the library in your Arduino sketches by adding `#include <DHT.h>` at the top of your sketch.

## Documentation

For detailed documentation, refer to the official documentation [here](https://docs.anedya.io/).

## License

This project is licensed under the [MIT License](https://github.com/anedyaio/anedya-example-nodemcu/blob/main/LICENSE).

> [!TIP]
> Looking for Python SDK? Visit [PyPi](https://pypi.org/project/anedya-dev-sdk/) or [Github Repository](https://github.com/anedyaio/anedya-dev-sdk-python)

>[!TIP]
> For more information, visit [anedya.io](https://anedya.io/?utm_source=github&utm_medium=link&utm_campaign=github-examples&utm_content=nodeMcu) 