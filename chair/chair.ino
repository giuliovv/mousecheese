#include <ESP8266WiFi.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;
int16_t gx_old, gy_old, gz_old;

int forward = 0, backward = 0, left = 0, right = 0;

const char *ssid = "chair";
const char *password = "password";
      

void setup() {
  Serial.begin(115200);
  delay(10);

  Wire.begin(D2, D1);
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

}

void loop() {
  // read raw accel/gyro measurements from device
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  gx_old = gx;
  gy_old = gy;
  gz_old = gz;

  // display tab-separated accel/gyro x/y/z values
  Serial.print("a/g:\t");
  Serial.print(ax); Serial.print("\t");
  Serial.print(ay); Serial.print("\t");
  Serial.print(az); Serial.print("\t");
  Serial.print(gx); Serial.print("\t");
  Serial.print(gy); Serial.print("\t");
  Serial.println(gz);

  if(ay>2000){
    forward = 1;
    backward = 0;
    delay(500);
  }
  if(ay<-2000) {
    backward = 1;
    forward = 0;
    delay(500);
  }
  Serial.print(forward); Serial.print("\t");
  Serial.println(backward);
  // gz_old-gz should be the difference in the angle between measurements, and that should detect turning of the wheelchair(gyro)
  // but I couldn't get the gyro to work and the values are not tested, so when you upload it to the esp just try to test a bit and fiddle with the values to get the right behaviour
  if(gz_old-gz>10000){
    left = 1;
    forward = backward = 0;
    delay(500);
  }
  if(gz-gz_old>10000){
    right = 1;
    forward = backward = 0;
    delay(500);
  }

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const char * host = "192.168.4.1";            //default IP address
  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request. Something like /data/?sensor_reading=123
  String url = "/data/";
  url += "?sensor_reading=";
  url +=  "{\"forward\":\"sensor0_value\",\"backward\":\"sensor1_value\",\"left\":\"sensor2_value\",\"right\":\"sensor3_value\"}";

  url.replace("sensor0_value", String(forward));
  url.replace("sensor1_value", String(backward));
  url.replace("sensor2_value", String(left));
  url.replace("sensor3_value", String(right));


  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
}
