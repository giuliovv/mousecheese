#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>

#include <ArduinoJson.h>
#include <DFPlayer_Mini_Mp3.h>

#define ENA D5
#define IN1 D6
#define IN2 D7
#define WHISKERS 3
#define PIN_BUSY D3
#define ENB D8
#define IN3 D0
#define IN4 D4

DynamicJsonBuffer jsonBuffer;

int whiskerread = 1;

const char *ssid      = "chair";
const char *password  = "password";

int forward = 0;        
int backward = 0;        
int left = 0;        
int right = 0;   
int stopp = 0;
int fixedlevel = 479;
int level = 0;
int forward_level = 0;        
int backward_level = 0; 
String sensor_values;

SoftwareSerial mp3Serial(D1, D2); // RX, TX

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
    forward = root["forward"].as<int>();
    backward = root["backward"].as<int>();
    left = root["left"].as<int>();
    right = root["right"].as<int>();
    stopp = root["stop"].as<int>();

  }

  Serial.println(forward);
  Serial.println(backward);
  Serial.println(left);
  Serial.println(right);
  Serial.println(stopp);

  level = level + (forward-backward);

  if(stopp==1){
    level = 0;
  }

  if (level > 4){ level = 4; };
  if (level < -4){ level = -4; };

  if (level == 0){
    fixedlevel = 0;
  } else {
    fixedlevel = 479;
  }
  if (level >= 0){
    forward_level = fixedlevel+level*133;
    backward_level = 0;
  } else {
    forward_level = 0;
    backward_level = fixedlevel-level*133;
  }

  server.send(200, "text/html", "OK");
}


void setup() {
  Serial.begin(115200);
  Serial.println("TEST");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();

  mp3Serial.begin (9600);
  mp3_set_serial (mp3Serial);

  delay(1000); 
  mp3_set_volume (100);

  // connect motor pins
  pinMode(ENA, OUTPUT); //ENA
  pinMode(IN1, OUTPUT); //IN1
  pinMode(IN2, OUTPUT); //IN2
  pinMode(ENB, OUTPUT); //ENB
  pinMode(IN3, OUTPUT); //IN3
  pinMode(IN4, OUTPUT); //IN4

  // Whisker
  pinMode(WHISKERS, INPUT);

  // Sound
  pinMode(PIN_BUSY, INPUT);
  
  server.on("/data/", HTTP_GET, handleSentVar); // when the server receives a request with /data/ in the string then run the handleSentVar function
  server.begin();
}

void loop() {
  server.handleClient();
  toggle_motors();
}

void toggle_motors()
{
  digitalWrite(ENA, HIGH);
  digitalWrite(ENB, HIGH);

  Serial.print("Forward: ");
  Serial.println(forward_level);
  Serial.print("Backward: ");
  Serial.println(backward_level);

  if(right != 1){
    analogWrite(IN1, forward_level);
    analogWrite(IN2, backward_level);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
  if(left != 1){
    analogWrite(IN3, forward_level);
    analogWrite(IN4, backward_level);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  whiskerread = digitalRead(WHISKERS);
  if (whiskerread == 0){
    Serial.println("TARGET");
    mp3_play();
  }

}
