#include <MPU6050.h>
#include <Wire.h>
#include <I2Cdev.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>  
#include <SoftwareSerial.h>

MPU6050 accelgyro;

#define UDP_PORT 5050
WiFiUDP UDP;
char message[255];

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t gx_old, gy_old, gz_old;

int forward = 0, backward = 0, left = 0, right = 0, stopp = 0;

const char *ssid = "chair";
const char *password = "password";

int lenave = 10;
int curravepos = 0;
int ave[10];

int zero_error = 0;

int olday = 0;
bool oldaction = false;

int level = 0;
int forward_level = 0;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  delay(10);
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  delay(10);
  
  UDP.begin(UDP_PORT);

  Wire.begin(D6, D7);
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  server.on("/", home); 
  server.on("/up", up);
  server.on("/down", down);
  server.on("/left", left_);
  server.on("/right", right_);
  server.on("/stop", stop);
  server.begin();

  for(int i = 0; i<100; i++){
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    zero_error += ay;
  }
  zero_error /= 100;
  Serial.println("Zero error: ");
  Serial.println(zero_error);

  for (int i=0; i< lenave; i++)  ave[i] = zero_error;
}

void home() {
  server.send(200, "text/html", "<a href='/forward' onclick='return true'>UP</a>    <a href='/down'>DOWN</a>   <a href='/left' onclick='return true'>LEFT</a>  <a href='/right' onclick='return true'>RIGHT</a><a href='/left' onclick='return true'>LEFT</a>  <a href='/stop' onclick='return true'>STOP</a>");
} 

void up(){
  forward = 1;
  server.send(200, "text/html", "<a href='/'>Indietro</a> Forward!");
}

void down(){
  backward = 1;
  server.send(200, "text/html", "<a href='/'>Indietro</a> Backward!");
}

void left_(){
  left = 1;
  server.send(200, "text/html", "<a href='/'>Indietro</a> Left!");
}

void right_(){
  right = 1;
  server.send(200, "text/html", "<a href='/'>Indietro</a> Right!");
}

void stop(){
  stopp = 1;
  server.send(200, "text/html", "<a href='/'>Indietro</a> Stop!");
}


void loop() {
  backward = forward = left = right = stopp = 0;
  server.handleClient();
  // read raw accel/gyro measurements from device
  
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  Serial.println(gz);

  // display tab-separated accel/gyro x/y/z values
  //Serial.print("a/g:\t");
  //Serial.print(ax/9.81); Serial.print("\t");
  //Serial.print(ay); Serial.print("\t");
  //Serial.print(az/9.81); Serial.print("\t");
  //Serial.print(gx); Serial.print("\t");
  //Serial.print(gy); Serial.print("\t");
  //Serial.println(gz);


  // adjust the zero error to new increasing error
  zero_error = zero_error * 0.99 + ay * 0.01;

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

  int res_adjusted = res - zero_error;

  /*if(res>-6500 && ! oldaction){
    backward = 1;
    Serial.println("BACK");
  }
  if(res<-9000 && ! oldaction) {
    forward = 1;
    Serial.println("FORWARD");
  }
  if(forward == 1 || backward == 1){
    oldaction = true;
    delay(300);
  } else {
    oldaction = false;
    olday = ay;
  }*/

  if(res_adjusted>1000 && ! oldaction){
    backward = 1;
    Serial.println("BACK");
  }
  if(res_adjusted<-1000 && ! oldaction) {
    forward = 1;
    Serial.println("FORWARD");
  }
  if(forward == 1 || backward == 1){
    oldaction = true;
    for (int i=0; i< lenave; i++)  ave[i] = zero_error;
    delay(300);
  } else {
    oldaction = false;
    olday = ay;
  }
 
  //Serial.print("RES:");
  //Serial.println(res);
  //Serial.print("RES ADJUSTED:");
  //Serial.println(res_adjusted);
  //Serial.print(forward);
  //Serial.print("\t");
  //Serial.println(backward);
  // gz_old-gz should be the difference in the angle between measurements, and that should detect turning of the wheelchair(gyro)
  // but I couldn't get the gyro to work and the values are not tested, so when you upload it to the esp just try to test a bit and fiddle with the values to get the right behaviour
  //Serial.println(gz);
  if(gz>10000){
    left = 1;
    Serial.print("left");
  }
  if(gz<-10000){
    right = 1;
    Serial.print("right");
  }
  //Serial.print("forward:");
  //Serial.println(forward);
  //Serial.print("backward:");
  //Serial.print("\t");
  //Serial.println(backward);
  //Serial.println("\t");
  //Serial.print(left);
  //Serial.print("\t");
  //Serial.println(right);

  String url = "{\"forward\":\"sensor0_value\",\"backward\":\"sensor1_value\",\"left\":\"sensor2_value\",\"right\":\"sensor3_value\",\"stop\":\"sensor_stop\"}";

  url.replace("sensor0_value", String(forward));
  url.replace("sensor1_value", String(backward));
  url.replace("sensor2_value", String(left));
  url.replace("sensor3_value", String(right));
  url.replace("sensor_stop", String(stopp));

  url.toCharArray(message, 255);
  
  UDP.beginPacket("192.168.4.2", 4210);
  UDP.write(message);
  UDP.endPacket();
  UDP.beginPacket("192.168.4.3", 4210);
  UDP.write(message);
  UDP.endPacket();
  delay(50);
}
