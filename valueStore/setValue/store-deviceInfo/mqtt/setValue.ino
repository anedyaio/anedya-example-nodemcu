/*

                                     Store Device Info - Example-ValueStore(mqtt)
                Disclaimer: This code is for hobbyists for learning purposes. Not recommended for production use!!

                            # Dashboard Setup
                             - Create account and login to the dashboard
                             - Create new project.
                             - Create a node (e.g., for home- Room1 or study room).
                            

                  
                                                                                           Dated: 16-April-2024
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>      // Ensure to include the ESP8266Wifi.h library, not the common library WiFi.
#include <PubSubClient.h>     //Include PubSubClient library to handle mqtt
#include <WiFiClientSecure.h> // include WiFiClientSecure to establish secure connect .. anedya only allow secure connection
#include <ArduinoJson.h> // Include the Arduino library to make json or abstract the value from the json
#include <TimeLib.h>     // Include the Time library to handle time synchronization with ATS (Anedya Time Services)

String regionCode = "ap-in-1";                   // Anedya region code (e.g., "ap-in-1" for Asia-Pacific/India) | For other country code, visity [https://docs.anedya.io/device/#region]
const char *PHYSICAL_DEVICE_ID = "<PHYSICAL-DEVICE-UUID>"; // Fill your device Id , that you can get from your node description
const char *CONNECTION_KEY = "<CONNECTION-KEY>";  // Fill your connection key, that you can get from your node description
const char *ssid = "<SSID>";     // Replace with your WiFi name
const char *pass = "<PASSWORD>"; // Replace with your WiFi password

// MQTT connection settings
String str_broker="mqtt."+String(regionCode)+".anedya.io";
const char *mqtt_broker = str_broker.c_str();                       // MQTT broker address
const char *mqtt_username = PHYSICAL_DEVICE_ID;                               // MQTT username
const char *mqtt_password = CONNECTION_KEY;                         // MQTT password
const int mqtt_port = 8883;                                       // MQTT port
String responseTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/response";  // MQTT topic for device responses
String errorTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/errors";  // MQTT topic for device errors



long long setValueTimer,updateInterval,timer, lastSubmittedHeartbeat_timer;   //timer variable for request handling
String valueRes;  //variable to store the response 


void connectToMQTT();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void anedya_setStrValue(String KEY, String VALUE);
void anedya_setBoolValue(String KEY, boolean VALUE);
void anedya_sendHeartbeat();


// WiFi and MQTT client initialization
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);


void setup()
{
  Serial.begin(115200); // Initialize serial communication with  your device compatible baud rate
  delay(1500);          // Delay for 1.5 seconds

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

  timer=millis();
  lastSubmittedHeartbeat_timer=millis();
 
esp_client.setInsecure();
mqtt_client.setServer(mqtt_broker, mqtt_port);  // Set the MQTT server address and port for the MQTT client to connect to anedya broker
mqtt_client.setKeepAlive(60);  // Set the keep alive interval (in seconds) for the MQTT connection to maintain connectivity
mqtt_client.setCallback(mqttCallback);  // Set the callback function to be invoked when MQTT messages are received
connectToMQTT(); // Attempt to establish a connection to the anedya broker
mqtt_client.setBufferSize(5000);
mqtt_client.subscribe(responseTopic.c_str());  //subscribe to get response
mqtt_client.subscribe(errorTopic.c_str());    //subscibe to get error

}

void loop()
{
  updateInterval=15000;
  if(millis()-timer>=updateInterval){

  String boardInfo = "Chip ID:" + String(ESP.getChipId(), HEX) +", "+
                   "CPU Frequency:" + String(ESP.getCpuFreqMHz()) +"MHz, "+
                   "Flash Size:" + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB" +", "+
                   "Free Heap Size:" + String(ESP.getFreeHeap()) +" bytes, "+
                   "Sketch Size:" + String(ESP.getSketchSize() / 1024) + " KB" +", "+
                   "Free Sketch Space:" + String(ESP.getFreeSketchSpace() / 1024) + " KB" +", "+
                   "Flash Speed:" + String(ESP.getFlashChipSpeed() / 1000000) + " MHz";

  anedya_setStrValue("009", boardInfo); /* anedya_setStrValue("<-KEY->","<-VALUE->")
                                                 1 parameter- key,                                                  
                                                 2 parameter- value.  For detailed info, visit-https://docs.anedya.io/valuestore/intro/        */
  anedya_setBoolValue("bool value", true); /* anedya_setBoolValue("<-KEY->","<-VALUE->")*/

   timer=millis();      
  }

  if(millis()-lastSubmittedHeartbeat_timer>=5000){
    anedya_sendHeartbeat();
    lastSubmittedHeartbeat_timer=millis();
  }
  mqtt_client.loop();
}
//<---------------------------------------------------------------------------------------------------------------------------->
void connectToMQTT()
{
  while (!mqtt_client.connected())
  {
    const char *client_id = PHYSICAL_DEVICE_ID;
    Serial.print("Connecting to Anedya Broker....... ");
    if (mqtt_client.connect(client_id, mqtt_username, mqtt_password))
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
     valueRes = str_res;
     Serial.println(valueRes);
}

void anedya_setStrValue(String KEY, String VALUE)
{
  boolean check = true;
  String strSetTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/valuestore/setValue/json";
  const char *setTopic = strSetTopic.c_str();
  while (check)
  {
    if (mqtt_client.connected())
    {
      if (millis() - setValueTimer >= 2000)
      {
        setValueTimer = millis();
       String valueJsonStr = "{\"reqId\": \"\",\"key\":\"" + KEY + "\",\"value\": \"" + VALUE + "\",\"type\": \"string\"}";
        const char *setValuePayload = valueJsonStr.c_str();
      mqtt_client.publish(setTopic, setValuePayload);
      }
      mqtt_client.loop();
      if (valueRes != "")
      {
        // Parse the JSON response
        DynamicJsonDocument jsonResponse(100);    // Declare a JSON document with a capacity of 200 bytes
        deserializeJson(jsonResponse, valueRes);  // Deserialize the JSON response from the server into the JSON document
        int errorCode = jsonResponse["errCode"];  // Get the server receive time from the JSON document
        if (errorCode == 0)
        {
          Serial.println("value set!!");
        }
        else
        {
          Serial.println("Failed to set value!");
          Serial.println(valueRes);
        }
        check = false;
        setValueTimer=5000;
        valueRes="";
      }
    }
    else
    {
      connectToMQTT();
    } // mqtt connect check end
  }
}


void anedya_setBoolValue(String KEY, boolean VALUE)
{
  boolean check = true;
  String strSetTopic = "$anedya/device/" + String(PHYSICAL_DEVICE_ID) + "/valuestore/setValue/json";
  const char *setTopic = strSetTopic.c_str();
  while (check)
  {
    if (mqtt_client.connected())
    {
      if (millis() - setValueTimer >= 2000)
      {
        setValueTimer = millis();
        String valueJsonStr = "{\"reqId\": \"\",\"key\":\"" + KEY + "\",\"value\": " + (VALUE ? "true" : "false") + ",\"type\": \"boolean\"}";
        const char *setValuePayload = valueJsonStr.c_str();
        mqtt_client.publish(setTopic, setValuePayload);
      }
      mqtt_client.loop();
      if (valueRes != "")
      {
        // Parse the JSON response
        DynamicJsonDocument jsonResponse(100);   // Declare a JSON document with a capacity of 200 bytes
        deserializeJson(jsonResponse, valueRes); // Deserialize the JSON response from the server into the JSON document
        int errorCode = jsonResponse["errCode"]; // Get the server receive time from the JSON document
        if (errorCode == 0)
        {
          Serial.println("value set!!");
        }
        else
        {
          Serial.println("Failed to set value!");
          Serial.println(valueRes);
        }
        check = false;
        setValueTimer = 5000;
        valueRes = "";
        submitRes = "";
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
