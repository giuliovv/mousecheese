#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>



#define led0 16               //D0
#define led1 5                //D1
#define led2 4                //D2
#define led3 0                //D3

DynamicJsonBuffer jsonBuffer;

const char *ssid      = "chair";
const char *password  = "password";

int sensorValue0 = 0;        
int sensorValue1 = 0;        
int sensorValue2 = 0;        
int sensorValue3 = 0;        
String sensor_values;

ESP8266WebServer server(80);

void handleSentVar() {

  if (server.hasArg("sensor_reading"))
  {
    sensor_values = server.arg("sensor_reading");
    Serial.println(sensor_values);
  }
  JsonObject& root = jsonBuffer.parseObject(sensor_values);
//  if (!root.success()) {
//    Serial.println("parseObject() failed");
//    return;
//  }
//  if (root.success())
//  {
    sensorValue0          = root["forward"].as<int>();
    sensorValue1          = root["backward"].as<int>();
    sensorValue2          = root["left"].as<int>();
    sensorValue3          = root["right"].as<int>();

//  }

  Serial.println(sensorValue0);
  Serial.println(sensorValue1);
  Serial.println(sensorValue2);
  Serial.println(sensorValue3);

  // toggle_leds();

  server.send(200, "text/html", "Data received");
}


void setup() {
  Serial.begin(9600);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();

  // pinMode(led0, OUTPUT);
  // pinMode(led1, OUTPUT);
  // pinMode(led2, OUTPUT);
  // pinMode(led3, OUTPUT);
  
  //toggle_leds();                 //turn off all leds as all the sensor values are zero
  
  server.on("/data/", HTTP_GET, handleSentVar); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.begin();
}

void loop() {
  server.handleClient();
}

void toggle_leds()
{
  if (sensorValue0 == 0)  digitalWrite(led0, LOW);
  if (sensorValue1 == 0)  digitalWrite(led1, LOW);
  if (sensorValue2 == 0)  digitalWrite(led2, LOW);
  if (sensorValue3 == 0)  digitalWrite(led3, LOW);

  if (sensorValue0 == 1)  digitalWrite(led0, HIGH);
  if (sensorValue1 == 1)  digitalWrite(led1, HIGH);
  if (sensorValue2 == 1)  digitalWrite(led2, HIGH);
  if (sensorValue3 == 1)  digitalWrite(led3, HIGH);
}
