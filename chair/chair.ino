#include <ESP8266WiFi.h>


#define btn0 16
#define btn1 5
#define btn2 4
#define btn3 0


const char *ssid = "chair";
const char *password = "password";


int sensorValue0 = 0;       //sensor value, I'm usingg 0/1 button state
int sensorValue1 = 0;        
int sensorValue2 = 0;        
int sensorValue3 = 0;       

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(btn0, INPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(btn3, INPUT);


  // set the ESP8266 to be a WiFi-client
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

}

void loop() {
  // if(digitalRead(btn0) == LOW) sensorValue0 = 1;
  // if(digitalRead(btn1) == LOW) sensorValue1 = 1;
  // if(digitalRead(btn2) == LOW) sensorValue2 = 1;
  // if(digitalRead(btn3) == LOW) sensorValue3 = 1;
  
  // if(digitalRead(btn0) == HIGH) sensorValue0 = 0;
  // if(digitalRead(btn1) == HIGH) sensorValue1 = 0;
  // if(digitalRead(btn2) == HIGH) sensorValue2 = 0;
  // if(digitalRead(btn3) == HIGH) sensorValue3 = 0;
  Serial.print("First: ");
  sensorValue0 = Serial.read();
  Serial.print("Second: ");
  sensorValue1 = Serial.read();
  Serial.print("Third: ");
  sensorValue2 = Serial.read();
  Serial.print("Fourth: ");
  sensorValue3 = Serial.read();


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

  url.replace("sensor0_value", String(sensorValue0));
  url.replace("sensor1_value", String(sensorValue1));
  url.replace("sensor2_value", String(sensorValue2));
  url.replace("sensor3_value", String(sensorValue3));


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
