# Room Monitoring - NodeMCU+DHT (Submit-Log)
[<img src="https://img.shields.io/badge/anedya-documentation-blue?link=https%3A%2F%2Fdocs.anedya.io">](https://docs.anedya.io)


This Arduino sketch allows you to submit your device log to the Anedya.

> [!WARNING]
> This code is for hobbyists for learning purposes. Not recommended for production use!!

## Set-Up

> [!IMPORTANT]
> Prerequisite: Set up the dashboard and fill the connection key and physical device ID in the firmware. For detailed guidance, visit [here](https://github.com/anedyaio/anedya-example-nodemcu/blob/main/README.md) 

### Code Set-Up 

To submit the the log, use : 

```
anedya_submitLog("<REQUEST-ID>","<LOG-MESSAGE>");
```

> [!TIP]
> Looking for Python SDK? Visit [PyPi](https://pypi.org/project/anedya-dev-sdk/) or [Github Repository](https://github.com/anedyaio/anedya-dev-sdk-pyhton)

>[!TIP]
> For more information, visit [anedya.io](https://anedya.io/)
 