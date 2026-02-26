/*

                            Room Monitoring Using NodeMCU + DHT11 sensor (mqtt)
                Disclaimer: This code is for hobbyists for learning purposes. Not recommended for production use!!

                            # Dashboard Setup
                             - create account and login to the dashboard
                             - Create project.
                             - Create a node (e.g., for home- Room1 or study room).
                             - Create variables: temperature and humidity.
                            Note: Variable Identifier is essential; fill it accurately.

                            # Hardware Setup
                             - Properly identify your sensor's pins.
                             - Connect sensor VCC pin to 3V3.
                             - Connect sensor GND pin to GND.
                             - Connect sensor signal pin to 5(marked D1 on the NodeMCu).

                  Note: The code is tested on the NodeMCU 1.0 board (ESP12E-Module)

                                                                                           Dated: 28-March-2024

*/
#include <Arduino.h>

// Emulate Hardware Sensor?
bool virtual_sensor = true;

#include <ESP8266WiFi.h>      // Ensure to include the ESP8266Wifi.h library, not the common library WiFi.
#include "time.h"             // Include time library to handle time
#include <PubSubClient.h>     //Include PubSubClient library to handle mqtt
#include <WiFiClientSecure.h> // include WiFiClientSecure to establish secure connect .. anedya only allow secure connection
#include <ArduinoJson.h>      // Include the Arduino library to make json or abstract the value from the json
#include <DHT.h>              // Include the DHT library for humidity and temperature sensor handling

// ----------------------------- Anedya and Wifi credentials --------------------------------------------
const char *CONNECTION_KEY = "REPLACE_WITH_YOUR_CONNECTION_KEY";         // Fill your connection key, that you can get from your node description
const char *PHYSICAL_DEVICE_ID = "REPLACE_WITH_YOUR_PHYSICAL_DEVICE_ID"; // Fill your device Id , that you can get from your node description
const char *SSID = "REPLACE_WITH_YOUR_SSID";
const char *PASSWORD = "REPLACE_WITH_YOUR_PASSWORD";
String REGION_CODE = "ap-in-1"; // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visity [https://docs.anedya.io/device/#region]
const char ANEDYA_ROOT_FINGERPRINT[] PROGMEM =
    "23 DE 98 81 16 9F 99 8F 86 AC 66 BF 9D B2 89 1D 18 31 67 F3"; // SHA1 fingerprint of Anedya Root CA 3 (ECC - 256)

// MQTT connection settings
String str_broker = "mqtt." + String(REGION_CODE) + ".anedya.io";
const char *mqtt_broker = str_broker.c_str();                                        // MQTT broker address
const char *mqtt_username = PHYSICAL_DEVICE_ID;                                      // MQTT username
const char *mqtt_password = CONNECTION_KEY;                                          // MQTT password
const int mqtt_port = 8883;                                                          // MQTT port
String responseTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/response"; // MQTT topic for device responses
String errorTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/errors";      // MQTT topic for device errors

long long submitTimer;     // timer to handle request delay
String timeRes, submitRes; // varibale to handle response

// Define the type of DHT sensor (DHT11, DHT21, DHT22, AM2301, AM2302, AM2321)
#define DHT_TYPE DHT11
// Define the pin connected to the DHT sensor
#define DHT_PIN 5 // pin marked as D1 on the NodeMCU board

// Define the temperature and humidity variables
float temperature;
float humidity;

// Function Declarations
void connectToMQTT();                                                                       // function to connect with the anedya broker
void mqttCallback(char *topic, byte *payload, unsigned int length);                         // funstion to handle call back
void anedya_submitData(String VARIABLE_IDENTIFIER, float sensor_data, long long timestamp); // Function to submit data to the Anedya server
void anedya_sendHeartbeat();

// WiFi and MQTT client initialization
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);

// Create a DHT object
DHT dht(DHT_PIN, DHT_TYPE);

void setup()
{
  Serial.begin(115200); // Initialize serial communication with  your device compatible baud rate
  delay(1500);          // Delay for 1.5 seconds

  // Connect to WiFi network
  WiFi.begin(SSID, PASSWORD);
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

  configTime(19800, 0, "time.anedya.io", "time.google.com", "pool.ntp.org"); // Configure time synchronization with NTP servers
  Serial.print("Waiting for time sync...");
  while ((long long)time(nullptr) < 1700000000)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("Done!");

  submitTimer = millis();

  // esp_client.setInsecure();
  esp_client.setFingerprint(ANEDYA_ROOT_FINGERPRINT);
  esp_client.setBufferSizes(512, 512);
  mqtt_client.setServer(mqtt_broker, mqtt_port); // Set the MQTT server address and port for the MQTT client to connect to anedya broker
  mqtt_client.setKeepAlive(60);                  // Set the keep alive interval (in seconds) for the MQTT connection to maintain connectivity
  mqtt_client.setCallback(mqttCallback);         // Set the callback function to be invoked when MQTT messages are received
  connectToMQTT();                               // Attempt to establish a connection to the anedya broker

  // Initialize the DHT sensor
  dht.begin();
}

void loop()
{

  if (!virtual_sensor)
  {
    // Read the temperature and humidity from the DHT sensor
    Serial.println("Fetching data from the Physical sensor");
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("Failed to read from DHT !"); // Output error message to serial console
      delay(10000);
      return;
    }
  }
  else
  {
    // Generate random temperature and humidity values
    Serial.println("Fetching data from the Virtual sensor");
    temperature = random(20, 30);
    humidity = random(60, 80);
  }
  Serial.print("Temperature : ");
  Serial.println(temperature);

  long long current_time = (long long)time(nullptr);  // Get the current time
  long long current_time_milli = current_time * 1000; // Convert current time to milliseconds

  // Submit sensor data to Anedya server
  anedya_submitData("temperature", temperature, current_time_milli); // submit data to the Anedya
  Serial.print("Humidity : ");
  Serial.println(humidity);

  anedya_submitData("humidity", humidity, current_time_milli); // submit data to the Anedya

  Serial.println("-------------------------------------------------");
  anedya_sendHeartbeat();
  delay(5000);
}
//<---------------------------------------------------------------------------------------------------------------------------->
void connectToMQTT()
{
  while (!mqtt_client.connected())
  {
    const char *client_id = PHYSICAL_DEVICE_ID;
    Serial.print("Connecting to Anedya Broker....... ");
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password)) // checks to check mqtt connection
    {
      mqtt_client.subscribe(responseTopic.c_str()); // subscribe to get response
      mqtt_client.subscribe(errorTopic.c_str());    // subscibe to get error
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
  else
  {
    // Serial.println(str_res);
    submitRes = str_res;
    Serial.println(str_res);
  }
}
// Function to submit data to Anedya server
// For more info, visit [https://docs.anedya.io/devicehttpapi/submitdata/]
void anedya_submitData(String VARIABLE_IDENTIFIER, float sensor_data, long long timestamp)
{
  boolean check = true;

  String strSubmitTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/submitdata/json";
  const char *submitTopic = strSubmitTopic.c_str();
  while (check)
  {
    if (mqtt_client.connected())
    {

      if (millis() - submitTimer >= 2000)
      {

        submitTimer = millis();

        // Construct the JSON payload with sensor data and timestamp
        String jsonStr = "{\"data\":[{\"variable\": \"" + VARIABLE_IDENTIFIER + "\",\"value\":" + String(sensor_data) + ",\"timestamp\":" + String(timestamp) + "}]}";
        Serial.println("Req Payload" + jsonStr);

        const char *submitJsonPayload = jsonStr.c_str();
        mqtt_client.publish(submitTopic, submitJsonPayload);
      }
      mqtt_client.loop();
      if (submitRes != "")
      {
        // Parse the JSON response
        DynamicJsonDocument jsonResponse(100);    // Declare a JSON document with a capacity of 200 bytes
        deserializeJson(jsonResponse, submitRes); // Deserialize the JSON response from the server into the JSON document

        int errorCode = jsonResponse["errorcode"]; // Get the server receive time from the JSON document
        if (errorCode == 0)
        {
          Serial.println("Data pushed to Anedya!!");
        }
        else if (errorCode == 4040)
        {
          Serial.println("Failed to push data!!");
          Serial.println("unknown variable Identifier");
          Serial.println(submitRes);
        }
        else
        {
          Serial.println("Failed to push data!!");
          Serial.println(submitRes);
        }
        check = false;
        submitTimer = 5000;
      }
    }
    else
    {
      connectToMQTT();
    } // mqtt connect check end
  }
}

void anedya_sendHeartbeat()
{
  mqtt_client.connected() ? (void)0 : connectToMQTT();

  String strHeartbeatTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/heartbeat/json";
  const char *heartbeatTopic = strHeartbeatTopic.c_str();

  String strPayload = "{}";
  const char *Payload = strPayload.c_str();
  mqtt_client.publish(heartbeatTopic, Payload);
  mqtt_client.loop();
}
