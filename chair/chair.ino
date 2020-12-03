#include <MPU6050.h>

#include <Wire.h>

#include <I2Cdev.h>
#include <ESP8266WiFi.h>

#include <WiFiUdp.h>  

MPU6050 accelgyro;

#define UDP_PORT 5050
WiFiUDP UDP;
char message[255];

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t gx_old, gy_old, gz_old;

int forward = 0, backward = 0, left = 0, right = 0;

const char *ssid = "chair";
const char *password = "password";

int lenave = 10;
int curravepos = 0;
int ave[10];
      

void setup() {
  Serial.begin(9600);
  delay(10);
  UDP.begin(UDP_PORT);

  Wire.begin(D6, D7);
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  // set the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected to The MouseTM");

}

void loop() {
  // read raw accel/gyro measurements from device
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  gx_old = gx;
  gy_old = gy;
  gz_old = gz;

  // display tab-separated accel/gyro x/y/z values
  //Serial.print("a/g:\t");
  //Serial.print(ax/9.81); Serial.print("\t");
  //Serial.print(ay); Serial.print("\t");
  //Serial.print(az/9.81); Serial.print("\t");
  //Serial.print(gx); Serial.print("\t");
  //Serial.print(gy); Serial.print("\t");
  //Serial.println(gz);

  ave[curravepos] = ay;
  curravepos += 1;
  if(curravepos == lenave){
    curravepos = 0;
  }
  int res = 0;
  for (int i=0; i< lenave; i++)
  {
    res += ave[i];
  }
  res = res/lenave;
  Serial.println(res);

  if(res<-1500){
    forward = 1;
  } else {
    forward = 0;
  }
  if(res>1000) {
    backward = 1;
  } else {
    backward = 0;
  }
  Serial.print(forward); Serial.print("\t");
  Serial.println(backward);
  // gz_old-gz should be the difference in the angle between measurements, and that should detect turning of the wheelchair(gyro)
  // but I couldn't get the gyro to work and the values are not tested, so when you upload it to the esp just try to test a bit and fiddle with the values to get the right behaviour
  if(gz_old-gz>10000){
    left = 1;
    //forward = backward = 0;
    //delay(50);
  } else {
    left = 0; 
  }
  if(gz-gz_old>10000){
    right = 1;
    //forward = backward = 0;
    //delay(50);
  } else {
    right = 0;
  }

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const char * host = "192.168.4.1";            //default IP address
  const int httpPort = 80;
  
  String url =  "{\"forward\":\"sensor0_value\",\"backward\":\"sensor1_value\",\"left\":\"sensor2_value\",\"right\":\"sensor3_value\"}";

  url.replace("sensor0_value", String(forward));
  url.replace("sensor1_value", String(backward));
  url.replace("sensor2_value", String(left));
  url.replace("sensor3_value", String(right));

  url.toCharArray(message, 255);

  UDP.beginPacket("192.168.4.1",5050);   
  UDP.write(message);
  UDP.endPacket();
}
