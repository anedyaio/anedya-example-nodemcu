/*

                           Store device Info - example-ValueStore  (http)
               Disclaimer: This code is for hobbyists for learning purposes. Not recommended for production use!!

                           # Dashboard Setup
                            - create account and login to the dashboard
                            - Create project.
                            - Create a node (e.g., for home- Room1 or study room).

                 Note: The code is tested on the NodeMCU 1.0 board (ESP12E-Module)
                  For more info, visit- https://docs.anedya.io/valuestore/intro
                                                                                          Dated: 8-April-2024

*/
#include <Arduino.h>

#include <ESP8266WiFi.h>       // Ensure to include the ESP8266Wifi.h library, not the common library WiFi.h
#include <ESP8266HTTPClient.h> // Ensure to include the ESP8266HTTPClient.h library, not the common library HTTPClient.h
#include <WiFiClient.h>
#include <ArduinoJson.h> // Include the ArduinoJson library for JSON handling
#include <TimeLib.h>     // Include the TimeLib library for time manipulation

String regionCode = "ap-in-1"; // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visity [https://docs.anedya.io/device/intro/#region]
String deviceID = "<PHYSICAL-DEVICE-UUID>";
String connectionKey = "<CONNECTION-KEY>";  // Fill your connection key, that you can get from your node description

// Your WiFi credentials
char ssid[] = "<SSID>";  // Your WiFi network SSID
char pass[] = "<PASSWORD>";  // Your WiFi network password

long long updateInterval,timer;   //varibles to insert interval

void anedya_setValue(String key, String type, String value);  //function declaration to set the value

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
    timer=millis();
}

void loop()
{
      updateInterval=5000;
  if(millis()-timer>=updateInterval){
String boardInfo = "Chip ID:" + String(ESP.getChipId(), HEX) +", "+
                   "CPU Frequency:" + String(ESP.getCpuFreqMHz()) +"MHz, "+
                   "Flash Size:" + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB" +", "+
                   "Free Heap Size:" + String(ESP.getFreeHeap()) +" bytes, "+
                   "Sketch Size:" + String(ESP.getSketchSize() / 1024) + " KB" +", "+
                   "Free Sketch Space:" + String(ESP.getFreeSketchSpace() / 1024) + " KB" +", "+
                   "Flash Speed:" + String(ESP.getFlashChipSpeed() / 1000000) + " MHz";

  anedya_setValue("001", "string", boardInfo); /* anedya_setValue("<-KEY->","<-dataType->","<-VALUE->")
                                                 1 parameter- key, 
                                                 2 parameter- The value can hold any of the following types of data: string, binary, float, boolean
                                                 3 parameter- value.  For detailed info, visit-https://docs.anedya.io/valuestore/intro/        */
   timer=millis();                             
  }
}
//<---------------------------------------------------------------------------------------------------------------------------->
void anedya_setValue(String KEY, String TYPE, String VALUE)
{
    if (WiFi.status() == WL_CONNECTED)
    {                            // Check if the device is connected to WiFi
        WiFiClientSecure client; // Initialize a secure WiFi client
        HTTPClient http;         // Initialize an HTTP client
        client.setInsecure();    // Configure the client to accept insecure connections
    String setValue_url = "https://device." + regionCode + ".anedya.io/v1/valuestore/setValue";  // Construct the URL for submitting data


        // Prepare data payload in JSON format
        http.begin(client, setValue_url);                   // Initialize the HTTP client with the Anedya server URL
        http.addHeader("Content-Type", "application/json"); // Set the content type of the request as JSON
        http.addHeader("Accept", "application/json");       // Specify the accepted content type
        http.addHeader("Auth-mode", "key");                 // Set authentication mode
        http.addHeader("Authorization", connectionKey);     // Add the connection key for authorization

    // Construct the JSON payload
    String valueJsonStr = "{\"reqId\": \"\",\"key\":\"" + KEY + "\",\"value\": \"" + VALUE + "\",\"type\": \"" + TYPE + "\"}";
        // Serial.println(jsonStr);
        // Send the POST request with the JSON payload to Anedya server
        int httpResponseCode = http.POST(valueJsonStr);

        // Check if the request was successful
        if (httpResponseCode > 0)
        {                                       // Successful response
            String response = http.getString(); // Get the response from the server
            // Parse the JSON response
            DynamicJsonDocument jsonSubmit_response(200);
            deserializeJson(jsonSubmit_response, response); // Extract the JSON response
                                                            // Extract the server time from the response
            int errorcode = jsonSubmit_response["errorcode"];
            if (errorcode == 0)
            { // error code  0 means data submitted successfull
                Serial.println("Value set!");
            }
            else
            { // other errocode means failed to push (like: 4020- mismatch variable identifier...)
                Serial.println("Failed to set!!");
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
