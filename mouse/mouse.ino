#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <WiFiUdp.h> 

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

int whiskerread = 1;

const char *ssid      = "chair";
const char *password  = "password";

int forward = 0;        
int backward = 0;        
int left = 0;        
int right = 0;   
int stopp = 0;
int fixedlevel = 679;
int level = 0;
int forward_level = 0;        
int backward_level = 0; 
//String sensor_values;

SoftwareSerial mp3Serial(D1, D2); // RX, TX

unsigned int localUdpPort = 4210;
WiFiUDP UDP;
char incomingPacket[255];

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();

  // set the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.println("Trying to connect ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected to TheChairTM");

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

  Serial.println(WiFi.localIP());
  
  UDP.begin(localUdpPort);
  Serial.print("UDP on:");
  Serial.println(localUdpPort);
}

void loop() {
  forward = backward = left = right = stopp = 0;
  int packetSize = UDP.parsePacket();
  if (packetSize){
    int len = UDP.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.print("Packet received: ");
    Serial.println(incomingPacket);
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(incomingPacket);
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
  }
  toggle_motors();
}

void toggle_motors()
{
  if (level > 3){ level = 3; };
  if (level < -3){ level = -3; };

  if (level == 0){
    fixedlevel = 0;
  } else {
    fixedlevel = 679;
  }
  if (level >= 0){
    forward_level = fixedlevel+level*133;
    backward_level = 0;
  } else {
    forward_level = 0;
    backward_level = fixedlevel-level*133;
  } 
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
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    delay(300);
  }
  if(left != 1){
    analogWrite(IN3, forward_level);
    analogWrite(IN4, backward_level);
  } else {
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
    delay(300);
  }

  whiskerread = digitalRead(WHISKERS);
  if (whiskerread == 0){
    Serial.println("TARGET");
    mp3_play();
    // DANCE
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
    analogWrite(IN2, 1000);
    analogWrite(IN4, 1000);
    delay(500);
    analogWrite(IN2, 0);
    delay(500);
    analogWrite(IN4, 0);
    analogWrite(IN1, 1000);
    analogWrite(IN3, 1000);
    delay(500);
    analogWrite(IN1, 0);
    analogWrite(IN2, 0);
    analogWrite(IN3, 0);
    analogWrite(IN4, 0);
  }

}
