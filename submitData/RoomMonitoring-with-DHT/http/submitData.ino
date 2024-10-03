/*

                            Room Monitoring Using NodeMCU + DHT11 sensor
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
                             - Connect sensor signal pin to 5(marked D1 on the NOdeMCU).

                  Note: The code is tested on the NodeMCU 1.0 board (ESP12E-Module)

                                                                                           Dated: 12-March-2024

*/
#include <Arduino.h>

// Emulate Hardware Sensor?
bool virtual_sensor = true;

#include <ESP8266WiFi.h>       // Ensure to include the ESP8266Wifi.h library, not the common library WiFi.h
#include <ESP8266HTTPClient.h> // Ensure to include the ESP8266HTTPClient.h library, not the common library HTTPClient.h
#include <WiFiClient.h>
#include <ArduinoJson.h> // Include the ArduinoJson library for JSON handling
#include <TimeLib.h>     // Include the TimeLib library for time manipulation
#include <DHT.h>         // Include the DHT library for humidity and temperature sensor handling

// ----------------------------- Anedya and Wifi credentials --------------------------------------------
String REGION_CODE = "ap-in-1";                   // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visity [https://docs.anedya.io/device/#region]
const char *CONNECTION_KEY = "";  // Fill your connection key, that you can get from your node description
const char *PHYSICAL_DEVICE_ID = ""; // Fill your device Id , that you can get from your node description
const char *SSID = "";     
const char *PASSWORD = ""; 

// Define the type of DHT sensor (DHT11, DHT21, DHT22, AM2301, AM2302, AM2321)
#define DHT_TYPE DHT11
// Define the pin connected to the DHT sensor
#define DHT_PIN 5 // pin marked as D1 on the nodemcu
float temperature;
float humidity;


// Function declarations
void setDevice_time();                                       // Function to configure the NodeMCU's time with real-time from ATS (Anedya Time Services)
void anedya_submitData(String datapoint, float sensor_data); // Function to submit data to the Anedya server
void anedya_sendHeartbeat();

// Create a DHT object
DHT dht(DHT_PIN, DHT_TYPE);

void setup()
{
  Serial.begin(115200); // Set the baud rate for serial communication

  // Connect to WiFi network
  WiFi.begin(SSID, PASSWORD);
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
      Serial.println("Failed to read from DHT !");       // Output error message to serial console
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

  anedya_sendHeartbeat();
  delay(5000);
}
//<---------------------------------------------------------------------------------------------------------------------------->
// Function to configure time synchronization with Anedya server
// For more info, visit [https://docs.anedya.io/device/api/http-time-sync/]
void setDevice_time()
{
  // URL to fetch real-time from Anedya server
  String time_url = "https://device." + REGION_CODE + ".anedya.io/v1/time";

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
    setTime(currentTimeSeconds);
  }
}

// Function to submit data to Anedya server
// For more info, visit [https://docs.anedya.io/devicehttpapi/submitdata/]
void anedya_submitData(String datapoint, float sensor_data)
{
  if (WiFi.status() == WL_CONNECTED)
  {                          // Check if the device is connected to WiFi
    WiFiClientSecure client; // Initialize a secure WiFi client
    HTTPClient http;         // Initialize an HTTP client
    client.setInsecure();    // Configure the client to accept insecure connections

    // Construct the URL for submitting data to Anedya server
    String sendData_url = "https://device." + REGION_CODE + ".anedya.io/v1/submitData";

    // Get current time and convert it to milliseconds
    long long current_time = now();                     // Get the current time from the device
    long long current_time_milli = current_time * 1000; // Convert current time to milliseconds

    // Prepare data payload in JSON format
    http.begin(client, sendData_url);                   // Initialize the HTTP client with the Anedya server URL
    http.addHeader("Content-Type", "application/json"); // Set the content type of the request as JSON
    http.addHeader("Accept", "application/json");       // Specify the accepted content type
    http.addHeader("Auth-mode", "key");                 // Set authentication mode
    http.addHeader("Authorization", CONNECTION_KEY);     // Add the connection key for authorization

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



//---------------------------------- Function for send heartbeat -----------------------------------
void anedya_sendHeartbeat()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client; // Initialize a secure WiFi client
    HTTPClient http;         // Initialize an HTTP client
    client.setInsecure();    // Configure the client to accept insecure connections                                                                  // Creating an instance of HTTPClient
    String heartbeat_url = "https://device." + REGION_CODE + ".anedya.io/v1/heartbeat"; // Constructing the URL for submitting data

    // Preparing data payload in JSON format
    http.begin( client,heartbeat_url);                           // Beginning an HTTP request to the specified URL
    http.addHeader("Content-Type", "application/json"); // Adding a header specifying the content type as JSON
    http.addHeader("Accept", "application/json");       // Adding a header specifying the accepted content type as JSON
    http.addHeader("Auth-mode", "key");                 // Adding a header specifying the authentication mode as "key"
    http.addHeader("Authorization", CONNECTION_KEY);     // Adding a header containing the authorization key

    // Constructing the JSON payload with sensor data and timestamp
    String body_payload = "{}";

    // Sending the POST request with the JSON payload to Anedya server
    int httpResponseCode = http.POST(body_payload);

    // Checking if the request was successful
    if (httpResponseCode > 0)
    {
      String response = http.getString(); // Getting the response from the server
      // Parsing the JSON response
      JsonDocument jsonSubmit_response;
      deserializeJson(jsonSubmit_response, response); // Extracting the JSON response
      int errorcode = jsonSubmit_response["errorcode"];
      if (errorcode == 0) // Error code 0 indicates data submitted successfully
      { 
        Serial.println("Sent Heartbeat");
      }
      else
      { 
        Serial.println("Failed to send heartbeat!!");
        Serial.println(response);  //error code4020 indicate -unknown variable identifier
      }   
    }                        
    else
    {
      Serial.print("Error on sending POST: "); // Printing error message indicating failure to send POST request
      Serial.println(httpResponseCode);        // Printing the HTTP response code
    }
    http.end(); // Ending the HTTP client session
  }
  else
  {
    Serial.println("Error in WiFi connection"); // Printing error message indicating WiFi connection failure
  }
}
