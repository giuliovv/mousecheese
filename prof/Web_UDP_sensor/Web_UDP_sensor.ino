#include <ESP8266WiFi.h>
#include <WiFiUdp.h>   
#include "Wire.h"
#include <ESP8266WebServer.h>
 
float acc_abs_old=0;
int x=0,counter=0,caduta=0,start_play,time_play=0;
int16_t tempo=0,tempo_old,fall=0;
const uint8_t MPU_addr=0x68; // I2C address of the MPU-6050
 
const float MPU_GYRO_250_SCALE = 131.0;
const float MPU_GYRO_500_SCALE = 65.5;
const float MPU_GYRO_1000_SCALE = 32.8;
const float MPU_GYRO_2000_SCALE = 16.4;
const float MPU_ACCL_2_SCALE = 16384.0;
const float MPU_ACCL_4_SCALE = 8192.0;
const float MPU_ACCL_8_SCALE = 4096.0;
const float MPU_ACCL_16_SCALE = 2048.0;

  
 
struct rawdata {
int16_t AcX;
int16_t AcY;
int16_t AcZ;
int16_t Tmp;
int16_t GyX;
int16_t GyY;
int16_t GyZ;
};
 
struct scaleddata{
float AcX;
float AcY;
float AcZ;
float Tmp;
float GyX;
float GyY;
float GyZ;
};
 
bool checkI2c(byte addr);
void mpu6050Begin(byte addr);
rawdata mpu6050Read(byte addr, bool Debug);
void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl);
void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl);
scaleddata convertRawToScaled(byte addr, rawdata data_in,bool Debug);

char tmp_str[7];                             /// funzione che converte inter_16 in stringhe 
char* convert_int16_to_str(int16_t i) { 
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

const char* ap_id   = "esp_ap";
const char* ap_pswd = "esp123456";
ESP8266WebServer server(80);

#define UDP_PORT 5050
WiFiUDP UDP;
char packet[255];

void setup() {
  tempo_old=millis();
  Wire.begin();
  Serial.begin(9600);
  WiFi.softAP(ap_id, ap_pswd);
  Serial.print("Connecting to ");
  Serial.println(ap_id);
  IPAddress ap_ip = WiFi.softAPIP();
  delay(2000); 
  Serial.begin(9600);
  Serial.println(ap_ip);
  mpu6050Begin(MPU_addr);
  UDP.begin(UDP_PORT);
  // Web-server //
  server.on("/", home); 
  server.on("/on", onpin);
  server.on("/off", offpin);
  server.on("/clear", clear_data);
  server.begin(); //avvia il server
}
 
void loop() {
  server.handleClient();
  rawdata next_sample;
  setMPU6050scales(MPU_addr,0b00000000,0b00011000);
  next_sample = mpu6050Read(MPU_addr, true);
  convertRawToScaled(MPU_addr, next_sample,true);
  tempo = millis () - tempo_old;
  tempo_old = millis();
  Serial.println(tempo);
  counter=counter+1;
}
// funzione che inizia la registrazione //
void onpin() {
  x=1;
  start_play=millis();
  //digitalWrite(13,HIGH);
  server.send(200, "text/html", "<a href='/'>Indietro</a> Hai iniziato la registrazione!");
}
// funzione che stoppa la registrazione //
void offpin() {
  x=0;
  time_play=millis()-start_play;
  Serial.print("Tempo giocato=  ");
  Serial.println(time_play/1000);
  UDP.beginPacket("192.168.4.2",5060);   
  UDP.write(convert_int16_to_str(time_play));
  UDP.endPacket();
  //digitalWrite(13,LOW);
  server.send(200, "text/html", "<a href='/'>Indietro</a> Hai terminato la registrazione!");  
} 
// azzera il numero di cadute //
void clear_data() {
  fall=0;
  server.send(200, "text/html", "<a href='/'>Indietro</a> Valori ripristinati!");  
  
}
// homepage 192.168.4.1 //
void home() {
//char* cadute = convert_int16_to_str(fall);
server.send(200, "text/html", "<a href='/on'>START</a>    <a href='/off'>FINISH</a>   <a href='/clear'>CLEAR DATA</a>  ");  
//server.send(200, "text/html", "<a href='/clear_data'>CLEAR DATA</a>");;
} 

void mpu6050Begin(byte addr){
// This function initializes the MPU-6050 IMU Sensor
// It verifys the address is correct and wakes up the
// MPU.
if (checkI2c(addr)){
Wire.beginTransmission(MPU_addr);
Wire.write(0x6B); // PWR_MGMT_1 register
Wire.write(0); // set to zero (wakes up the MPU-6050)
Wire.endTransmission(true);
 
delay(30); // Ensure gyro has enough time to power up
}
}
 
bool checkI2c(byte addr){
// We are using the return value of
// the Write.endTransmisstion to see if
// a device did acknowledge to the address.
Serial.println(" ");
Wire.beginTransmission(addr);
 
if (Wire.endTransmission() == 0)
{
Serial.print(" Device Found at 0x");
Serial.println(addr,HEX);
return true;
}
else
{
Serial.print(" No Device Found at 0x");
Serial.println(addr,HEX);
return false;
}
}
 

rawdata mpu6050Read(byte addr, bool Debug){
// This function reads the raw 16-bit data values from
// the MPU-6050
 
rawdata values;
 
Wire.beginTransmission(addr);
Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
Wire.endTransmission(false);
Wire.requestFrom(addr,14,true); // request a total of 14 registers
values.AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
values.AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
values.AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
values.Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
values.GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
values.GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
values.GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
 
 
if(Debug){
/*Serial.print(" GyX = "); Serial.print(values.GyX);
Serial.print(" | GyY = "); Serial.print(values.GyY);
Serial.print(" | GyZ = "); Serial.print(values.GyZ);
Serial.print(" | Tmp = "); Serial.print(values.Tmp);
Serial.print(" | AcX = "); Serial.print(values.AcX);
Serial.print(" | AcY = "); Serial.print(values.AcY);
Serial.print(" | AcZ = "); Serial.println(values.AcZ);*/

}
 
return values;
}
 
void setMPU6050scales(byte addr,uint8_t Gyro,uint8_t Accl){
Wire.beginTransmission(addr);
Wire.write(0x1B); // write to register starting at 0x1B
Wire.write(Gyro); // Self Tests Off and set Gyro FS to 250
Wire.write(Accl); // Self Tests Off and set Accl FS to 8g
Wire.endTransmission(true);
}
 
void getMPU6050scales(byte addr,uint8_t &Gyro,uint8_t &Accl){
Wire.beginTransmission(addr);
Wire.write(0x1B); // starting with register 0x3B (ACCEL_XOUT_H)
Wire.endTransmission(false);
Wire.requestFrom(addr,2,true); // request a total of 14 registers
Gyro = (Wire.read()&(bit(3)|bit(4)))>>3;
Accl = (Wire.read()&(bit(3)|bit(4)))>>3;
}
 
scaleddata convertRawToScaled(byte addr, rawdata data_in, bool Debug){

scaleddata values;
float scale_value = 0.0;
byte Gyro, Accl;
 
getMPU6050scales(MPU_addr, Gyro, Accl);
 
if(Debug){
//Serial.print("Gyro Full-Scale = ");
}
 
switch (Gyro){
case 0:
scale_value = MPU_GYRO_250_SCALE;
if(Debug){
//Serial.println("±250 °/s");
}
break;
case 1:
scale_value = MPU_GYRO_500_SCALE;
if(Debug){
Serial.println("±500 °/s");
}
break;
case 2:
scale_value = MPU_GYRO_1000_SCALE;
if(Debug){
Serial.println("±1000 °/s");
}
break;
case 3:
scale_value = MPU_GYRO_2000_SCALE;
if(Debug){
//Serial.println("±2000 °/s");
}
break;
default:
break;
}
 
values.GyX = (float) data_in.GyX / scale_value;
values.GyY = (float) data_in.GyY / scale_value;
values.GyZ = (float) data_in.GyZ / scale_value;
 
scale_value = 0.0;
if(Debug){
//Serial.print("Accl Full-Scale = ");
}
switch (Accl){
case 0:
scale_value = MPU_ACCL_2_SCALE;
if(Debug){
Serial.println("±2 g");
}
break;
case 1:
scale_value = MPU_ACCL_4_SCALE;
if(Debug){
Serial.println("±4 g");
}
break;
case 2:
scale_value = MPU_ACCL_8_SCALE;
if(Debug){
Serial.println("±8 g");
}
break;
case 3:
scale_value = MPU_ACCL_16_SCALE;
if(Debug){
//Serial.println("±16 g");
}
break;
default:
break;
}
values.AcX = (float) data_in.AcX / scale_value;
values.AcY = (float) data_in.AcY / scale_value;
values.AcZ = (float) data_in.AcZ / scale_value;
 
  
 
values.Tmp = (float) data_in.Tmp / 340.0 + 36.53;
 
if(Debug){
/*Serial.print(" GyX = "); Serial.print(values.GyX);
Serial.print(" °/s| GyY = "); Serial.print(values.GyY);
Serial.print(" °/s| GyZ = "); Serial.print(values.GyZ);
Serial.print(" °/s| Tmp = "); Serial.print(values.Tmp);
Serial.print(" °C| AcX = "); Serial.print(values.AcX);
Serial.print(" g| AcY = "); Serial.print(values.AcY);
Serial.print(" g| AcZ = "); Serial.print(values.AcZ);Serial.println(" g");*/

char acc_x[0],acc_y[0],acc_z[0],acc_ass[0];
float acc_abs,acc_abs_old;

acc_abs = sqrt(values.AcX*values.AcX + values.AcY*values.AcY + values.AcZ*values.AcZ);

// counter delle cadute della palla //
if(acc_abs>8 && counter-caduta>1000){
  fall=fall+1;  
  caduta=counter;  
}
//invio dati tramite UDP se la registrazione è in corso//
if(x==1) {
UDP.beginPacket("192.168.4.2",5060);   /// invia pacchetti all'inidirizzo specifico, UDP.remoteIP invia all'indirizzo che ha spedito
//UDP.write(convert_int16_to_str(tempo));
//UDP.write(" ");
UDP.write(convert_int16_to_str(fall));
UDP.write(" ");
UDP.write(dtostrf(values.AcX,0,2,acc_x));
UDP.write(" ");
UDP.write(dtostrf(values.AcY,0,2,acc_y));
UDP.write(" ");
UDP.write(dtostrf(values.AcZ,0,2,acc_z));
UDP.write(" ");
UDP.write(dtostrf(acc_abs,0,2,acc_ass));
UDP.endPacket(); 
}
}

return values;
}
