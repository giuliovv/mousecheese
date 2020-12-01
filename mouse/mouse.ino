#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>


DynamicJsonBuffer jsonBuffer;

const char *ssid      = "chair";
const char *password  = "password";

int forward = 0;        
int backward = 0;        
int left = 0;        
int right = 0;   
int stopp = 0;
int forward_level = 0;        
int backward_level = 0; 
String sensor_values;

ESP8266WebServer server(80);

void handleSentVar() {

  if (server.hasArg("sensor_reading"))
  {
    sensor_values = server.arg("sensor_reading");
    Serial.println(sensor_values);
  }
  JsonObject& root = jsonBuffer.parseObject(sensor_values);
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  if (root.success())
  {
    forward          = root["forward"].as<int>();
    backward          = root["backward"].as<int>();
    left          = root["left"].as<int>();
    right          = root["right"].as<int>();
    stopp          = root["stop"].as<int>();

  }

  Serial.println(forward);
  Serial.println(backward);
  Serial.println(left);
  Serial.println(right);
  Serial.println(stopp);

  forward_level = forward_level + (forward-backward);
  backward_level = backward_level + (backward-forward);

  if (forward_level > 4){ forward_level = 4 };
  if (forward_level < 0){ forward_level = 0 };
  if (backward_level > 4){ backward_level = 4 };
  if (backward_level < 0){ backward_level = 0 };

  server.send(200, "text/html", "OK");
}


void setup() {
  Serial.begin(9600);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();

  // connect motor pins
  pinMode(D1, OUTPUT); //IN1
  pinMode(D2, OUTPUT); //IN2
  pinMode(D0, OUTPUT); //ENA
  pinMode(D6, OUTPUT); //IN3
  pinMode(D7, OUTPUT); //IN4
  pinMode(D5, OUTPUT); //ENB
  
  server.on("/data/", HTTP_GET, handleSentVar); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.begin();
}

void loop() {
  server.handleClient();
  toggle_motors();
}

void toggle_motors()
{
  digitalWrite(D0, HIGH);
  digitalWrite(D5, HIGH);

  if (forward == 1)  {
    digitalWrite(D1, HIGH);
    digitalWrite(D2, LOW);
    digitalWrite(D6, HIGH);
    digitalWrite(D7, LOW);
  } else if(backward == 1){
    digitalWrite(D1, LOW);
    digitalWrite(D2, HIGH);
    digitalWrite(D6, LOW);
    digitalWrite(D7, HIGH);
  } else if(left == 1) {
    digitalWrite(D1, HIGH);
    digitalWrite(D2, LOW);
    digitalWrite(D6, LOW);
    digitalWrite(D7, LOW);
  } else if(right == 1) {
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(D6, HIGH);
    digitalWrite(D7, LOW);
  } else if(stopp == 1) {
    digitalWrite(D1, LOW);
    digitalWrite(D2, LOW);
    digitalWrite(D6, LOW);
    digitalWrite(D7, LOW);
  }
}
