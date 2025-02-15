/*

                                              Basic Home Automation with Anedya
                Disclaimer: This code is for hobbyists for learning purposes. Not recommended for production use!!

                            # Dashboard Setup
                             - create account and login to the dashboard
                             - Create project.
                             - Create a node (e.g., for home:-Room1 or study room).
                             - Create variables: temperature and humidity.
                            Note: Variable Identifier is essential; fill it accurately.

                            # Hardware Setup
                             -Add relay for the fan at pin D5
                             -Add relay for the light at pin D0
                             -Add buzzer at pin D6
                             -Add dht sensor at pin D1

                  Note: The code is tested on the NodeMCU 1.0 board (ESP12E-Module)

                                                                                           Dated: 7-June-2024

*/
#include <Arduino.h>
#include <ESP8266WiFi.h>      // Ensure to include the ESP8266Wifi.h library, not the common library WiFi.
#include <PubSubClient.h>     //Include PubSubClient library to handle mqtt
#include <WiFiClientSecure.h> // include WiFiClientSecure to establish secure connect .. anedya only allow secure connection
#include <ArduinoJson.h>      // Include the Arduino library to make json or abstract the value from the json
#include <TimeLib.h>          // Include the Time library to handle time synchronization with ATS (Anedya Time Services)
#include <DHT.h>              // Include the DHT library for humidity and temperature sensor handling

//-----------------------------------Variable section----------------------------------------------------------------------------------
//-------------------------------------Controllers------------------------------------------------------------
#define dhtData_submission_interval_ms 120000 // It will submit the data at the interval of 2 min

// ----------------------------- Anedya and Wifi credentials --------------------------------------------
String REGION_CODE = "ap-in-1";                                          // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visity [https://docs.anedya.io/device/#region]
const char *CONNECTION_KEY = "";         // Fill your connection key, that you can get from your node description
const char *PHYSICAL_DEVICE_ID = ""; // Fill your device Id , that you can get from your node description
const char *SSID = "";
const char *PASSWORD = "";

//-------------------------------Pins allocation---------------------------------------------------------------
#define lightPin 16  // pin marked as D0 on the NodeMCU board
#define fanPin 14    // pin marked as D5 on the NodeMCU board
#define buzzerPin 12 // pin marked as D6 on the NodeMCU board
#define DHT_PIN 5    // pin marked as D1 on the NodeMCU board
//------------------------------------------------------------------------------------------------------------
#define DHT_TYPE DHT11 // Define the type of DHT sensor (DHT11, DHT21, DHT22, AM2301, AM2302, AM2321)
//---------------------------------Mqtt-variables----------------------------------------------------------------
// MQTT connection settings
String str_broker = "mqtt." + String(REGION_CODE) + ".anedya.io";
const char *mqtt_broker = str_broker.c_str();                                                        // MQTT broker address
const char *mqtt_username = PHYSICAL_DEVICE_ID;                                                      // MQTT username
const char *mqtt_password = CONNECTION_KEY;                                                          // MQTT password
const int mqtt_port = 8883;                                                                          // MQTT port
String responseTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/response";                 // MQTT topic for device responses
String errorTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/errors";                      // MQTT topic for device errors
String commandTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/commands";                  // MQTT topic for device commands
String statusTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/commands/updateStatus/json"; // MQTT topic update status of the command

//------------------------------------Helper variables-----------------------------------------------------------
String timeRes, submitRes, commandId, fan_commandId, light_commandId;
long long submitTimer, submitLogTimer, submitDataTimer, submitTemHum_timer, lastSubmittedHeartbeat_timestamp; // timer's variable to control the flow
String fanStatus = "on";                                                                                      // variable to store the status of the fan
String lightStatus = "off";                                                                                   // variable to store the status of the light
float temperature;
float humidity;
//------------------------------------------- client initialization------------------------------------------------
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);

DHT dht(DHT_PIN, DHT_TYPE);

//------------------------------------------Function Decalaration section----------------------------------------
void connectToMQTT();                                               // function to connect with the anedya broker
void mqttCallback(char *topic, byte *payload, unsigned int length); // function to handle call back
void setDevice_time();                                              // Function to configure the device time with real-time from ATS (Anedya Time Services)
void anedya_submitData(String VARIABLE_IDENTIFIER, float sensor_data);
void anedya_submitLog(String reqID, String Log);
void anedya_updateCommand_status(String COMMAND_ID, String STATUS);
void beep(unsigned int DELAY);
void anedya_sendHeartbeat();

//-------------------------------------------------Setup-----------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200); // Initialize serial communication with  your device compatible baud rate
  delay(1500);          // Delay for 1.5 seconds

  // Connect to WiFi network
  WiFi.mode(WIFI_STA);
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

  submitLogTimer = millis(); // assigning value to handle interval
  submitDataTimer = millis();
  submitTemHum_timer = millis();
  lastSubmittedHeartbeat_timestamp = millis();

  esp_client.setInsecure();
  mqtt_client.setServer(mqtt_broker, mqtt_port); // Set the MQTT server address and port for the MQTT client to connect to anedya broker
  mqtt_client.setKeepAlive(60);                  // Set the keep alive interval (in seconds) for the MQTT connection to maintain connectivity
  mqtt_client.setCallback(mqttCallback);         // Set the callback function to be invoked when MQTT messages are received
  connectToMQTT();                               // Attempt to establish a connection to the anedya broker

  setDevice_time(); // function to sync the the device time
  dht.begin();

  pinMode(fanPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  anedya_submitLog("", "Device Restarted!!"); // it will submit log to the anedya that device is started
}
//--------------------------------------------------LOOP-----------------------------------------------------------------------------------------------
void loop()
{

  if (!mqtt_client.connected())
  {
    connectToMQTT();
  }
  if (millis() - submitTemHum_timer >= dhtData_submission_interval_ms)
  { //
    submitTemHum_timer = millis();
    Serial.println("Fetching data from the Physical sensor");
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    if (isnan(humidity) || isnan(temperature))
    {
      Serial.println("Failed to read from DHT !"); // Output error message to serial console
    }
    else
    {
      Serial.print("Temperature : ");
      Serial.println(temperature);
      anedya_submitData("temperature", temperature); // submit data to the Anedya

      Serial.print("Humidity : ");
      Serial.println(humidity);

      anedya_submitData("humidity", humidity); // submit data to the Anedya
    }
  }

  //--------------------------------------------------------fan-------------------------------
  if (fan_commandId != "")
  {
    if (fanStatus == "on" || fanStatus == "ON")
    {
      digitalWrite(fanPin, HIGH); // turn on the fan
      Serial.println("Fan ON");
      beep(250);
      anedya_updateCommand_status(fan_commandId, "success");
      anedya_submitLog("", "Fan ON");
    }
    else if (fanStatus == "off" || fanStatus == "OFF")
    {
      digitalWrite(fanPin, LOW);
      Serial.println("Fan OFF");
      beep(250);
      anedya_updateCommand_status(fan_commandId, "success");
      anedya_submitLog("", "Fan OFF");
    }
    else
    {

      anedya_updateCommand_status(fan_commandId, "failure");
      Serial.println("Invalid Command");
    }
    fan_commandId = ""; // checks
  }

  //--------------------------------------------------------light------------------------------
  if (light_commandId != "")
  { // condition block to publish the command success or failure message
    if (lightStatus == "on" || lightStatus == "ON")
    {
      digitalWrite(lightPin, HIGH);
      beep(250);
      Serial.println("light ON");

      anedya_updateCommand_status(light_commandId, "success");
      anedya_submitLog("", "light ON");
    }
    else if (lightStatus == "off" || lightStatus == "OFF")
    {
      digitalWrite(lightPin, LOW);
      Serial.println("light OFF");
      beep(250);
      anedya_updateCommand_status(light_commandId, "success");
      anedya_submitLog("", "light OFF");
    }
    else
    {
      anedya_updateCommand_status(light_commandId, "failure");
      Serial.println("Invalid Command");
      anedya_submitLog("", "Invalid Command"); // submit log in the case of the unknown or invalid command
    }
    light_commandId = ""; // flow checks
  }
  if (millis() - lastSubmittedHeartbeat_timestamp >= 5000)
  {
    anedya_sendHeartbeat();
    lastSubmittedHeartbeat_timestamp = millis();
  }

  mqtt_client.loop();
}

//<-------------------------------------------------Function Section------------------------------------------------------------------------------------->
//---------------------------------------MQTT connection function----------------------------------------------
void connectToMQTT()
{
  while (!mqtt_client.connected())
  {
    const char *client_id = PHYSICAL_DEVICE_ID;
    Serial.print("Connecting to Anedya Broker....... ");
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password)) // checks to check mqtt connection
    {
      Serial.println("Connected to Anedya broker");
      mqtt_client.subscribe(responseTopic.c_str()); // subscribe to get response
      mqtt_client.subscribe(errorTopic.c_str());    // subscibe to get error
      mqtt_client.subscribe(commandTopic.c_str());
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
//-------------------------------------------MQTT call back function--------------------------------------------
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  // Serial.print("Message received on topic: ");
  // Serial.println(topic);
  char res[150] = "";
  for (unsigned int i = 0; i < length; i++)
  {
    res[i] = payload[i];
  }
  String str_res(res);
  // Serial.println(str_res);
  JsonDocument Response;
  deserializeJson(Response, str_res);
  if (Response["deviceSendTime"]) // block to get the device send time
  {
    timeRes = str_res;
  }
  else if (Response["command"]) // block to get the command
  {
    commandId = String(Response["commandId"]);
    String equipment = Response["command"].as<String>();
    anedya_updateCommand_status(commandId, "received");
    anedya_submitLog("", "command received");
    if (equipment == "fan" or equipment == "Fan")
    {
      fanStatus = String(Response["data"]);
      fan_commandId = String(Response["commandId"]);
    }
    else if (equipment == "light" or equipment == "Light")
    {
      lightStatus = String(Response["data"]);
      light_commandId = String(Response["commandId"]);
    }
    else
    {
      anedya_updateCommand_status(commandId, "failure");
      anedya_submitLog("", "unknown command!!");
      beep(3000);
    }
  }
  else if (String(Response["errorcode"]) == "0")
  {
    submitRes = str_res;
  }
  else // block to debug errors
  {
    Serial.println(str_res);
  }
}

//------------------------------------------------------------------Time synchronization function---------------------------
// Function to configure time synchronization  with Anedya server
// For more info, visit [https://docs.anedya.io/device/api/http-time-sync/]
void setDevice_time()
{
  String timeTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/time/json"; // time topic wil provide the current time from the anedya server
  const char *mqtt_topic = timeTopic.c_str();
  // Attempt to synchronize time with Anedya server
  if (mqtt_client.connected())
  {

    Serial.print("Time synchronizing......");

    boolean timeCheck = true; // iteration to re-sync to ATS (Anedya Time Services), in case of failed attempt

    long long deviceSendTime;
    long long timeTimer = millis();
    while (timeCheck)
    {
      mqtt_client.loop();

      unsigned int iterate = 2000;
      if (millis() - timeTimer >= iterate)
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
      }

      if (timeRes != "")
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
  }
  else
  {
    connectToMQTT();
  } // mqtt connect check end
} // set device time function end

//-----------------------------------------------------------submitdata--------------------------------------------------------------------
// Function to submit data to Anedya server
// For more info, visit [https://docs.anedya.io/devicehttpapi/submitdata/]
void anedya_submitData(String VARIABLE_IDENTIFIER, float sensor_data)
{
  boolean check = true;

  String strSubmitTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/submitdata/json";
  const char *submitTopic = strSubmitTopic.c_str();

  while (check)
  {
    if (mqtt_client.connected())
    {

      if (millis() - submitDataTimer >= 2000)
      {
        submitDataTimer = millis();
        // Get current time and convert it to milliseconds
        long long current_time = now();                     // Get the current time
        long long current_time_milli = current_time * 1000; // Convert current time to milliseconds

        // Construct the JSON payload with sensor data and timestamp
        String jsonStr = "{\"data\":[{\"variable\": \"" + VARIABLE_IDENTIFIER + "\",\"value\":" + String(sensor_data) + ",\"timestamp\":" + String(current_time_milli) + "}]}";
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
        submitDataTimer = 5000;
      }
    }
    else
    {
      connectToMQTT();
    } // mqtt connect check end
  }
}
//-------------------------------------------------------Function to submit log---------------------------------------------------------------
void anedya_submitLog(String reqID, String Log)
{
  boolean check = true;
  String strSubmitTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/logs/submitLogs/json";
  const char *submitTopic = strSubmitTopic.c_str();
  while (check)
  {
    if (mqtt_client.connected())
    {
      if (millis() - submitLogTimer >= 2000)
      {
        submitLogTimer = millis();
        // Get current time and convert it to milliseconds
        long long current_time = now();                     // Get the current time
        long long current_time_milli = current_time * 1000; // Convert current time to milliseconds
        String strLog = "{\"reqId\":\"" + reqID + "\",\"data\":[{\"timestamp\":" + String(current_time_milli) + ",\"log\":\"" + Log + "\"}]}";
        const char *submitLogPayload = strLog.c_str();
        mqtt_client.publish(submitTopic, submitLogPayload);
      }
      mqtt_client.loop();
      if (submitRes != "")
      {
        // Parse the JSON response
        DynamicJsonDocument jsonResponse(100);    // Declare a JSON document with a capacity of 200 bytes
        deserializeJson(jsonResponse, submitRes); // Deserialize the JSON response from the server into the JSON document
        int errorCode = jsonResponse["errorcode"];  // Get the server receive time from the JSON document
        if (errorCode == 0)
        {
          Serial.println("Log pushed to Anedya!!");
        }
        else
        {
          Serial.println("Failed to push!");
          Serial.println(submitRes);
        }
        check = false;
        submitLogTimer = 5000;
      }
    }
    else
    {
      connectToMQTT();
    } // mqtt connect check end
  }
}
//------------------------------------------------Buzzer--------------------------------------------------------------------
void beep(unsigned int DELAY)
{
  digitalWrite(buzzerPin, HIGH);
  delay(DELAY);
  digitalWrite(buzzerPin, LOW);
}
//-------------------------------------------------update_command_status--------------------------------------------------

void anedya_updateCommand_status(String COMMAND_ID, String STATUS)
{
  String statusPayload = "{\"reqId\": \"\",\"commandId\": \"" + COMMAND_ID + "\",\"status\": \"" + STATUS + "\",\"ackdata\": \"\",\"ackdatatype\": \"\"}";
  mqtt_client.publish(statusTopic.c_str(), statusPayload.c_str());
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