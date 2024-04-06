# Anedya-Example-NodeMcu

[<img src="https://img.shields.io/badge/Anedya-Documentation-blue?link=https%3A%2F%2Fdocs.anedya.io">](https://docs.anedya.io)

<p align="center">
    <img src="https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcRmAlgiLhWiZb81tWYDrQ4ApVKXPyjuuN3XQMwgPJKJUhTmAVx5XOrkzZECCKgzl0i43g&usqp=CAU" alt="Logo">
</p>
This repository contains example projects for the ESP8266(NodeMcu) microcontroller utilizing the Anedya APIs. Anedya is a comprehensive cloud platform that offers Platform as a Service (PaaS) for IoT applications. It covers all aspects, including device lifecycle management, data storage, alerts, and data aggregation in a single platform. Anedya simplifies the integration of IoT hardware with the server and eliminates the burden of IoT cloud infrastructure development and management.

## Examples Included:

The examples demonstrate how to utilize Anedya with NodeMcu:

- **Data Submission and Visualization via HTTP:** Submit data to the Anedya cloud and visualize it using HTTP requests. Click [here](https://github.com/anedyaio/anedya-example-nodemcu/blob/main/submitData/Room%20Monitoring%20with%20DHT/http/README.md), for more info.
   
- **Data Submission and Visualization via MQTT:** Submit data to the Anedya cloud and visualize it using MQTT (Message Queuing Telemetry Transport) protocol. Click [here](https://github.com/anedyaio/anedya-example-nodemcu/blob/main/submitData/Room%20Monitoring%20with%20DHT/mqtt/README.md), for more info.
   
- **Device Logs Submission via HTTP:** Submit device logs to the Anedya cloud using HTTP requests. Click [here](https://github.com/anedyaio/anedya-example-nodemcu/blob/main/submitLogs/http/README.md), for more info.
   
- **Device Logs Submission via MQTT:** Submit device logs to the Anedya cloud using MQTT. click [here](https://github.com/anedyaio/anedya-example-nodemcu/blob/main/submitLogs/mqtt/README.md), for more info.
   
- **Device Commands Control via MQTT:** Control your device through the anedya dashboard-commands using MQTT. Click [here](https://github.com/anedyaio/anedya-example-nodemcu/blob/main/commands/mqtt/README.md), for more info.


## Getting Started

To get started with the examples:

1. Choose the code, create the .ino file, and open it in the Arduino IDE.
2. Fill in your Wi-Fi credentials, physical device ID, and connection key (obtained from the dashboard).
3. Select the Board and Port number.
4. Upload the code.

## Documentation

For detailed documentation, refer to the official documentation [here](https://docs.anedya.io/).

## License

This project is licensed under the MIT License.

> [!TIP]
> Looking for Python SDK? Visit [PyPi](https://pypi.org/project/anedya-dev-sdk/) or [Github Repository](https://github.com/anedyaio/anedya-dev-sdk-pyhton)

>[!TIP]
> For more information, visit [anedya.io](https://anedya.io/)
