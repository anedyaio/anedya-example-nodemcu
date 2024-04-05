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

#include <ESP8266WiFi.h>       // Ensure to include the ESP8266Wifi.h library, not the common library WiFi.
#include <PubSubClient.h>      //Include PubSubClient library to handle mqtt
#include <WiFiClientSecure.h>  // include WiFiClientSecure to establish secure connect .. anedya only allow secure connection
#include <ArduinoJson.h>       // Include the Arduino library to make json or abstract the value from the json
#include <TimeLib.h>           // Include the Time library to handle time synchronization with ATS (Anedya Time Services)

String regionCode = "ap-in-1";                                   // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visity [https://docs.anedya.io/device/intro/#region]
const char *deviceID = "281edd1c-ef6c-4fd3-aa32-d5b07a53f982";   // Fill your device Id , that you can get from your node description
const char *connectionkey = "0685b02726aa96438e16e53a8e11425b";  // Fill your connection key, that you can get from your node description
// WiFi credentials
const char *ssid = "Invesun_2.4GHz";  // Replace with your WiFi name
const char *pass = "Invesun123com#";  // Replace with your WiFi password

// MQTT connection settings
const char *mqtt_broker = "device.ap-in-1.anedya.io";                       // MQTT broker address
const char *mqtt_username = deviceID;                                       // MQTT username
const char *mqtt_password = connectionkey;                                  // MQTT password
const int mqtt_port = 8883;                                                 // MQTT port
String responseTopic = "$anedya/device/" + String(deviceID) + "/response";  // MQTT topic for device responses
String errorTopic = "$anedya/device/" + String(deviceID) + "/errors";       // MQTT topic for device errors
String commandTopic = "$anedya/device/" + String(deviceID) + "/commands";

String statusTopic = "$anedya/device/" + String(deviceID) + "/commands/updateStatus/json";
String timeRes, commandId;  // varibale to handle response
String ledStatus = "off";
long long responseTimer = 0;
  bool processCheck=false;

const int ledPin = 2;  //

// Function Declarations
void connectToMQTT();                                                // function to connect with the anedya broker
void mqttCallback(char *topic, byte *payload, unsigned int length);  // funstion to handle call back
void setDevice_time();                                               // Function to configure the device time with real-time from ATS (Anedya Time Services)

// WiFi and MQTT client initialization
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);

void setup() {
  Serial.begin(115200);  // Initialize serial communication with  your device compatible baud rate
  delay(1500);           // Delay for 1.5 seconds

  // Connect to WiFi network
  WiFi.begin(ssid, pass);
  Serial.println();
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());



  esp_client.setInsecure();
  mqtt_client.setServer(mqtt_broker, mqtt_port);  // Set the MQTT server address and port for the MQTT client to connect to anedya broker
  mqtt_client.setKeepAlive(60);                   // Set the keep alive interval (in seconds) for the MQTT connection to maintain connectivity
  mqtt_client.setCallback(mqttCallback);          // Set the callback function to be invoked when MQTT messages are received
  connectToMQTT();                                // Attempt to establish a connection to the anedya broker
  mqtt_client.subscribe(responseTopic.c_str());   // subscribe to get response
  mqtt_client.subscribe(errorTopic.c_str());      // subscibe to get error
  mqtt_client.subscribe(commandTopic.c_str());

  setDevice_time();  // function to sync the the device time

  // Initialize the built-in LED pin as an output
  pinMode(ledPin, OUTPUT);
}

void loop() {

  if (!mqtt_client.connected()) {
    connectToMQTT();
  }
  if (millis() - responseTimer > 700 && processCheck && commandId != "") {
    String statusProcessingPayload="{\"reqId\": \"\",\"commandId\": \"" + commandId +"\",\"status\": \"processing\",\"ackdata\": \"\",\"ackdatatype\": \"\"}";
    mqtt_client.publish(statusTopic.c_str(), statusProcessingPayload.c_str());
    Serial.println("Processing..");
    processCheck=false;

  } else if (millis() - responseTimer >= 1000 && commandId != "") {

    if (ledStatus == "on" || ledStatus == "ON") {
      // Turn the LED on (HIGH)
      digitalWrite(ledPin, HIGH);
      Serial.println("Led ON");
    } else {
      // Turn the LED off (LOW)
      digitalWrite(ledPin, LOW);
      Serial.println("Led OFF");
    }
    String statusSuccessPayload="{\"reqId\": \"\",\"commandId\": \"" + commandId +"\",\"status\": \"success\",\"ackdata\": \"\",\"ackdatatype\": \"\"}";
    mqtt_client.publish(statusTopic.c_str(), statusSuccessPayload.c_str());
    commandId = "";
  }

  mqtt_client.loop();
}
//<---------------------------------------------------------------------------------------------------------------------------->
void connectToMQTT() {
  while (!mqtt_client.connected()) {
    const char *client_id = deviceID;
    Serial.print("Connecting to Anedya Broker....... ");
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password))  // checks to check mqtt connection
    {
      Serial.println("Connected to Anedya broker");
    } else {
      Serial.print("Failed to connect to Anedya broker, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Retrying in 5 seconds.");
      delay(5000);
    }
  }
}
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  // Serial.print("Message received on topic: ");
  // Serial.println(topic);
  char res[150] = "";
  for (unsigned int i = 0; i < length; i++) {
    // Serial.print((char)payload[i]);
    //   Serial.print(payload[i]);
    res[i] = payload[i];
  }
  String str_res(res);
  JsonDocument Response;
  deserializeJson(Response, str_res);
  if (Response["deviceSendTime"]) {
    timeRes = str_res;
  } else if (Response["command"]) {

    ledStatus = String(Response["data"]);  // Get the server receive time from the JSON document
    responseTimer = millis();
    commandId = String(Response["id"]);
    Serial.println(commandId);
    String statusReceivedPayload="{\"reqId\": \"\",\"commandId\": \"" + commandId +"\",\"status\": \"received\",\"ackdata\": \"\",\"ackdatatype\": \"\"}";
    mqtt_client.publish(statusTopic.c_str(), statusReceivedPayload.c_str());
    processCheck=true;
        Serial.println(str_res);


    Serial.println("Response Receiving");
  } else {
    Serial.println(str_res);
  }
}
// Function to configure time synchronization with Anedya server
// For more info, visit [https://docs.anedya.io/devicehttpapi/http-time-sync/]
void setDevice_time() {
  String timeTopic = "$anedya/device/" + String(deviceID) + "/time/json";  // time topic wil provide the current time from the anedya server
  const char *mqtt_topic = timeTopic.c_str();
  // Attempt to synchronize time with Anedya server
  if (mqtt_client.connected()) {

    Serial.print("Time synchronizing......");

    boolean timeCheck = true;  // iteration to re-sync to ATS (Anedya Time Services), in case of failed attempt
    // Get the device send time

    long long deviceSendTime;
    long long timeTimer = millis();
    while (timeCheck) {
      mqtt_client.loop();

      unsigned int iterate = 2000;
      if (millis() - timeTimer >= iterate)  // time to hold publishing
      {
        Serial.print(".");
        timeTimer = millis();
        deviceSendTime = millis();

        // Prepare the request payload
        StaticJsonDocument<200> requestPayload;             // Declare a JSON document with a capacity of 200 bytes
        requestPayload["deviceSendTime"] = deviceSendTime;  // Add a key-value pair to the JSON document
        String jsonPayload;                                 // Declare a string to store the serialized JSON payload
        serializeJson(requestPayload, jsonPayload);         // Serialize the JSON document into a string
        // Convert String object to pointer to a string literal
        const char *jsonPayloadLiteral = jsonPayload.c_str();
        mqtt_client.publish(mqtt_topic, jsonPayloadLiteral);

      }  // if end

      if (timeRes != "")  // processed it got response
      {
        String strResTime(timeRes);

        // Parse the JSON response
        DynamicJsonDocument jsonResponse(100);      // Declare a JSON document with a capacity of 200 bytes
        deserializeJson(jsonResponse, strResTime);  // Deserialize the JSON response from the server into the JSON document

        long long serverReceiveTime = jsonResponse["serverReceiveTime"];  // Get the server receive time from the JSON document
        long long serverSendTime = jsonResponse["serverSendTime"];        // Get the server send time from the JSON document

        // Compute the current time
        long long deviceRecTime = millis();                                                                 // Get the device receive time
        long long currentTime = (serverReceiveTime + serverSendTime + deviceRecTime - deviceSendTime) / 2;  // Compute the current time
        long long currentTimeSeconds = currentTime / 1000;                                                  // Convert current time to seconds

        // Set device time
        setTime(currentTimeSeconds);  // Set the device time based on the computed current time
        Serial.println("\n synchronized!");
        timeCheck = false;
      }  // response check
    }
    // while loop end
  } else {
    connectToMQTT();
  }  // mqtt connect check end
}  // set device time function end
