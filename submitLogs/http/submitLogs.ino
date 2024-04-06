/*

                            Room Monitoring Using NodeMCU + DHT11 sensor With Device log submission
                Disclaimer: This code is for hobbyists for learning purposes. Not recommended for production use!!

                            # Dashboard Setup
                             - create account and login to the dashboard
                             - Create project.
                             - Create a node (e.g., for home- Room1 or study room).
                             - Create variables: temperature and humidity.
                            Note: Variable Identifier is essential; fill it accurately.(Caution: Case-Sensitive)

                            # Hardware Setup
                             - Properly identify your sensor's pins.
                             - Connect sensor VCC pin to 3V3.
                             - Connect sensor GND pin to GND.
                             - Connect sensor signal pin to 5(marked D1 on the nodemcu).

                  Note: The code is tested on the NodeMCU 1.0 board (ESP12E-Module)

                                                                                           Dated: 12-March-2024

*/
#include <Arduino.h>

// Emulate Hardware Sensor?
bool virtual_sensor = false;

#include <ESP8266WiFi.h>       // Ensure to include the ESP8266Wifi.h library, not the common library WiFi.h
#include <ESP8266HTTPClient.h> // Ensure to include the ESP8266HTTPClient.h library, not the common library HTTPClient.h
#include <WiFiClient.h>
#include <ArduinoJson.h> // Include the ArduinoJson library for JSON handling
#include <TimeLib.h>     // Include the TimeLib library for time manipulation
#include <DHT.h>         // Include the DHT library for humidity and temperature sensor handling

String regionCode = "ap-in-1"; // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visity [https://docs.anedya.io/device/intro/#region]
String deviceID = "<PHYSICAL-DEVICE-UUID>";
String connectionkey = "<CONNECTION-KEY>"; // Fill your connection key, that you can get from your node description

// Define the type of DHT sensor (DHT11, DHT21, DHT22, AM2301, AM2302, AM2321)
#define DHT_TYPE DHT11
// Define the pin connected to the DHT sensor
#define DHT_PIN 5 // pin marked as D1 on the nodemcu

float temperature; // float variable to strore the sensor data
float humidity;    // float variable to store the sensor data

char ssid[] = "<SSID>";     // Your WiFi network SSID
char pass[] = "<PASSWORD>"; // Your WiFi network password

// Function declarations
void setDevice_time();                                       // Function to configure the NodeMCU's time with real-time from ATS (Anedya Time Services)
void anedya_submitData(String datapoint, float sensor_data); // Function to submit data to the Anedya server
void anedya_submitLog(String reqID, String Log);            // Function to submit Log to the Anedya server

// Create a DHT object
DHT dht(DHT_PIN, DHT_TYPE);

void setup()
{
  Serial.begin(115200); // Set the baud rate for serial communication

  // Connect to WiFi network
  WiFi.begin(ssid, pass);
  Serial.println();
  Serial.print("[SETUP] Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  { // Wait until connected
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to ATS(Anedya Time Services) and configure time synchronization
  setDevice_time(); // Call function to configure time synchronization

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
      static int counter;
      counter++;
      String deviceLog = "[" + String(counter) + "] Failed to read from DHT !";
      Serial.println(deviceLog);       // Output error message to serial console
      anedya_submitLog("", deviceLog); // anedya_submitLog("<--request ID-->", "<--Log-->"); you can keep request id empty also
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
  // Submit sensor data to Anedya server
  anedya_submitData("temperature", temperature); // submit data to the Anedya

  Serial.print("Humidity : ");
  Serial.println(humidity);
  anedya_submitData("humidity", humidity); // submit data to the Anedya

  delay(15000);
}
//<---------------------------------------------------------------------------------------------------------------------------->
// Function to configure time synchronization with Anedya server
// For more info, visit [https://docs.anedya.io/devicehttpapi/http-time-sync/]
void setDevice_time()
{
  // URL to fetch real-time from Anedya server
  String time_url = "https://device." + regionCode + ".anedya.io/v1/time";

  // Attempt to synchronize time with Anedya server
  Serial.println("Time synchronizing......");
  int timeCheck = 1; // Iteration control
  while (timeCheck)
  {
    long long deviceSendTime = millis(); // Get the current device running time

    // Prepare the request payload
    StaticJsonDocument<200> requestPayload;
    requestPayload["deviceSendTime"] = deviceSendTime;
    String jsonPayload;
    serializeJson(requestPayload, jsonPayload);

    WiFiClientSecure client; // Anedya works only on HTTPS, so make sure to use WiFiClientSecure, not only WiFiClient
    HTTPClient http;         // Initialize an HTTP client
    client.setInsecure();

    // Send a POST request to fetch server time
    http.begin(client, time_url);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonPayload);

    // Check if the request was successful
    if (httpResponseCode != 200)
    {
      Serial.println("Failed to fetch server time!");
      delay(2000);
    }
    else if (httpResponseCode == 200)
    {
      timeCheck = 0;
      Serial.println("synchronized!");
    }

    // Parse the JSON response
    DynamicJsonDocument jsonResponse(200);
    deserializeJson(jsonResponse, http.getString()); // Extract the JSON response

    // Extract the server time from the response
    long long serverReceiveTime = jsonResponse["serverReceiveTime"];
    long long serverSendTime = jsonResponse["serverSendTime"];

    // Compute the current time
    long long deviceRecTime = millis();
    long long currentTime = (serverReceiveTime + serverSendTime + deviceRecTime - deviceSendTime) / 2;
    long long currentTimeSeconds = currentTime / 1000;

    // Set device time
   // Serial.println(currentTimeSeconds);

    setTime(currentTimeSeconds);
  }
}

// Function to submit Log to Anedya server
// For more info, visit [https://docs.anedya.io/devicehttpapi/submitdata/]
void anedya_submitData(String datapoint, float sensor_data)
{
  if (WiFi.status() == WL_CONNECTED)
  {                          // Check if the device is connected to WiFi
    WiFiClientSecure client; // Initialize a secure WiFi client
    HTTPClient http;         // Initialize an HTTP client
    client.setInsecure();    // Configure the client to accept insecure connections

    // Construct the URL for submitting data to Anedya server
    String senddata_url = "https://device." + regionCode + ".anedya.io/v1/submitData";

    // Get current time and convert it to milliseconds
    long long current_time = now();                     // Get the current time from the device
    long long current_time_milli = current_time * 1000; // Convert current time to milliseconds

    http.begin(client, senddata_url);                   // Initialize the HTTP client with the Anedya server URL
    http.addHeader("Content-Type", "application/json"); // Set the content type of the request as JSON
    http.addHeader("Accept", "application/json");       // Specify the accepted content type
    http.addHeader("Auth-mode", "key");                 // Set authentication mode
    http.addHeader("Authorization", connectionkey);     // Add the connection key for authorization

    // Construct the JSON payload with sensor data and timestamp
    String jsonStr = "{\"data\":[{\"variable\": \"" + datapoint + "\",\"value\":" + String(sensor_data) + ",\"timestamp\":" + String(current_time_milli) + "}]}";

    // Serial.println(jsonStr);
    // Send the POST request with the JSON payload to Anedya server
    int httpResponseCode = http.POST(jsonStr);

    // Check if the request was successful
    if (httpResponseCode > 0)
    {                                     // Successful response
      String response = http.getString(); // Get the response from the server

      // Parse the JSON response
      DynamicJsonDocument jsonSubmit_response(200);
      deserializeJson(jsonSubmit_response, response); // Extract the JSON response

      // Extract the server time from the response
      int errorcode = jsonSubmit_response["errorcode"];
      if (errorcode == 0)
      { // error code  0 means data submitted successfull
        Serial.println("Data pushed to Anedya Cloud!");
      }
      else
      { // other errocode means failed to push (like: 4020- mismatch variable identifier...)
        Serial.println("Failed to push!!");
        Serial.println(response); // Print the response
      }
    }
    else
    { // Error handling for failed request
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode); // Print the HTTP response code
    }
    http.end(); // End the HTTP client session
  }
  else
  { // Error handling for WiFi connection failure
    Serial.println("Error in WiFi connection");
  }
}

// Function to submit data to Anedya server
// For more info, visit [https://docs.anedya.io/devicehttpapi/logs-submit-logs/]
void anedya_submitLog(String reqID, String Log)
{
  if (WiFi.status() == WL_CONNECTED)
  {                          // Check if the device is connected to WiFi
    WiFiClientSecure client; // Initialize a secure WiFi client
    HTTPClient http;         // Initialize an HTTP client
    client.setInsecure();    // Configure the client to accept insecure connections

    // Construct the URL for submitting log to Anedya
    String sendLogs_url = "https://device." + regionCode + ".anedya.io/v1/logs/submitLogs";

    // Get current time and convert it to milliseconds
    long long current_time = now();                     // Get the current time from the device
    long long current_time_milli = current_time * 1000; // Convert current time to milliseconds

    http.begin(client, sendLogs_url);                   // Initialize the HTTP client with the Anedya server URL
    http.addHeader("Content-Type", "application/json"); // Set the content type of the request as JSON
    http.addHeader("Accept", "application/json");       // Specify the accepted content type
    http.addHeader("Auth-mode", "key");                 // Set authentication mode
    http.addHeader("Authorization", connectionkey);     // Add the connection key for authorization

    // Construct the JSON payload with deveice log and timestamp
    String strLog = "{\"reqId\":\"" + reqID + "\",\"data\":[{\"timestamp\":" + String(current_time_milli) + ",\"log\":\"" + Log + "\"}]}";

    // Send the POST request with the JSON payload to Anedya server
    int httpResponseCode = http.POST(strLog);

    // Check if the request was successful
    if (httpResponseCode > 0)
    {                                     // Successful response
      String response = http.getString(); // Get the response from the server

      // Parse the JSON response
      DynamicJsonDocument jsonSubmit_response(200);
      deserializeJson(jsonSubmit_response, response); // Extract the JSON response

      // Extract the server time from the response
      int errorcode = jsonSubmit_response["errorcode"]; // to find from the  response
      if (errorcode == 0)
      { // error code  0 means data submitted successfull
         Serial.println("Log pushed to Anedya Cloud!");
        //Serial.println(response); // Print the response
      }
      else
      {
        Serial.println("Failed to push!!");
        Serial.println(response); // Print the response
      }
    }
    else
    { // Error handling for failed request
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode); // Print the HTTP response code
    }

    http.end(); // End the HTTP client session
  }
  else
  { // Error handling for WiFi connection failure
    Serial.println("Error in WiFi connection");
  }
}
