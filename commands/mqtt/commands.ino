/*

                                            Smart Home (mqtt)
                Disclaimer: This code is for hobbyists for learning purposes. Not recommended for production use!!

                            # Dashboard Setup
                             - create account and login to the dashboard
                             - Create project.
                             - Create a node (e.g., for home- Room1 or study room).
                             - Create variables: temperature and humidity.
                            Note: Variable Identifier is essential; fill it accurately.

                            # Hardware Setup

                  Note: The code is tested on the NodeMCU 1.0 board (ESP12E-Module)

                                                                                           Dated: 28-March-2024

*/
#include <Arduino.h>

#include <ESP8266WiFi.h>      // Ensure to include the ESP8266Wifi.h library, not the common library WiFi.
#include <PubSubClient.h>     //Include PubSubClient library to handle mqtt
#include <WiFiClientSecure.h> // include WiFiClientSecure to establish secure connect .. anedya only allow secure connection
#include <ArduinoJson.h>      // Include the Arduino library to make json or abstract the value from the json
#include <TimeLib.h>          // Include the Time library to handle time synchronization with ATS (Anedya Time Services)

String regionCode = "ap-in-1";                   // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visity [https://docs.anedya.io/device/intro/#region]
const char *deviceID = "<PHYSICAL-DEVICE-UUID>"; // Fill your device Id , that you can get from your node description
const char *connectionkey = "<CONNECTION-KEY>";  // Fill your connection key, that you can get from your node description
// WiFi credentials
const char *ssid = "<SSID>";     // Replace with your WiFi name
const char *pass = "<PASSWORD>"; // Replace with your WiFi password

// MQTT connection settings
const char *mqtt_broker = "device.ap-in-1.anedya.io";                      // MQTT broker address
const char *mqtt_username = deviceID;                                      // MQTT username
const char *mqtt_password = connectionkey;                                 // MQTT password
const int mqtt_port = 8883;                                                // MQTT port
String responseTopic = "$anedya/device/" + String(deviceID) + "/response"; // MQTT topic for device responses
String errorTopic = "$anedya/device/" + String(deviceID) + "/errors";      // MQTT topic for device errors

String timeRes, command; // varibale to handle response
Srting ledStatus = "off";

const int ledPin = LED_BUILDIN;

// Function Declarations
void connectToMQTT();                                               // function to connect with the anedya broker
void mqttCallback(char *topic, byte *payload, unsigned int length); // funstion to handle call back
void setDevice_time();                                              // Function to configure the device time with real-time from ATS (Anedya Time Services)

// WiFi and MQTT client initialization
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);

void setup()
{
  Serial.begin(115200); // Initialize serial communication with  your device compatible baud rate
  delay(1500);          // Delay for 1.5 seconds

  // Connect to WiFi network
  WiFi.begin(ssid, pass);
  Serial.println();
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  submitTimer = millis();

  esp_client.setInsecure();
  mqtt_client.setServer(mqtt_broker, mqtt_port); // Set the MQTT server address and port for the MQTT client to connect to anedya broker
  mqtt_client.setKeepAlive(60);                  // Set the keep alive interval (in seconds) for the MQTT connection to maintain connectivity
  mqtt_client.setCallback(mqttCallback);         // Set the callback function to be invoked when MQTT messages are received
  connectToMQTT();                               // Attempt to establish a connection to the anedya broker
  mqtt_client.subscribe(responseTopic.c_str());  // subscribe to get response
  mqtt_client.subscribe(errorTopic.c_str());     // subscibe to get error

  setDevice_time(); // function to sync the the device time

  // Initialize the built-in LED pin as an output
  pinMode(ledPin, OUTPUT);
}

void loop()
{

  if (!mqtt_client.connected())
  {
    connectToMQTT();
  }

  if (ledStatus == "on" || ledStatus == "ON")
  {
    // Turn the LED on (HIGH)
    digitalWrite(ledPin, HIGH);
    Serial.println("Led ON");
  }
  else
  {
    // Turn the LED off (LOW)
    digitalWrite(ledPin, LOW);
    Serial.println("Led OFF");
  }

  mqtt_client.loop();
}
//<---------------------------------------------------------------------------------------------------------------------------->
void connectToMQTT()
{
  while (!mqtt_client.connected())
  {
    const char *client_id = deviceID;
    Serial.print("Connecting to Anedya Broker....... ");
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password)) // checks to check mqtt connection
    {
      Serial.println("Connected to Anedya broker");
    }
    else
    {
      Serial.print("Failed to connect to Anedya broker, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Retrying in 5 seconds.");
      delay(5000);
    }
  }
}
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  // Serial.print("Message received on topic: ");
  // Serial.println(topic);
  char res[150] = "";
  for (unsigned int i = 0; i < length; i++)
  {
    // Serial.print((char)payload[i]);
    //   Serial.print(payload[i]);
    res[i] = payload[i];
  }
  String str_res(res);
  if (str_res.indexOf("deviceSendTime") != -1)
  {
    timeRes = str_res;
  }
  else if (str_res.indexOf("<-command indentifier->") != -1)
  {

    // Parse the JSON response
    DynamicJsonDocument jsonCommands(100);          // Declare a JSON document with a capacity of 200 bytes
    deserializeJson(jsonCommands, str_res);         // Deserialize the JSON response from the server into the JSON document
    ledStatus = jsonCommands["<-command status->"]; // Get the server receive time from the JSON document
  }
  else
  {
    // Serial.println(str_res);
    submitRes = str_res;
    Serial.println(str_res);
  }
}
// Function to configure time synchronization with Anedya server
// For more info, visit [https://docs.anedya.io/devicehttpapi/http-time-sync/]
void setDevice_time()
{
  String timeTopic = "$anedya/device/" + String(deviceID) + "/time/json"; // time topic wil provide the current time from the anedya server
  const char *mqtt_topic = timeTopic.c_str();
  // Attempt to synchronize time with Anedya server
  if (mqtt_client.connected())
  {

    Serial.print("Time synchronizing......");

    boolean timeCheck = true; // iteration to re-sync to ATS (Anedya Time Services), in case of failed attempt
    // Get the device send time

    long long deviceSendTime;
    long long timeTimer = millis();
    while (timeCheck)
    {
      mqtt_client.loop();

      unsigned int iterate = 2000;
      if (millis() - timeTimer >= iterate) // time to hold publishing
      {
        Serial.print(".");
        timeTimer = millis();
        deviceSendTime = millis();

        // Prepare the request payload
        StaticJsonDocument<200> requestPayload;            // Declare a JSON document with a capacity of 200 bytes
        requestPayload["deviceSendTime"] = deviceSendTime; // Add a key-value pair to the JSON document
        String jsonPayload;                                // Declare a string to store the serialized JSON payload
        serializeJson(requestPayload, jsonPayload);        // Serialize the JSON document into a string
        // Convert String object to pointer to a string literal
        const char *jsonPayloadLiteral = jsonPayload.c_str();
        mqtt_client.publish(mqtt_topic, jsonPayloadLiteral);

      } // if end

      if (timeRes != "") // processed it got response
      {
        String strResTime(timeRes);

        // Parse the JSON response
        DynamicJsonDocument jsonResponse(100);     // Declare a JSON document with a capacity of 200 bytes
        deserializeJson(jsonResponse, strResTime); // Deserialize the JSON response from the server into the JSON document

        long long serverReceiveTime = jsonResponse["serverReceiveTime"]; // Get the server receive time from the JSON document
        long long serverSendTime = jsonResponse["serverSendTime"];       // Get the server send time from the JSON document

        // Compute the current time
        long long deviceRecTime = millis();                                                                // Get the device receive time
        long long currentTime = (serverReceiveTime + serverSendTime + deviceRecTime - deviceSendTime) / 2; // Compute the current time
        long long currentTimeSeconds = currentTime / 1000;                                                 // Convert current time to seconds

        // Set device time
        setTime(currentTimeSeconds); // Set the device time based on the computed current time
        Serial.println("\n synchronized!");
        timeCheck = false;
      } // response check
    }
    // while loop end
  }
  else
  {
    connectToMQTT();
  } // mqtt connect check end
} // set device time function end
