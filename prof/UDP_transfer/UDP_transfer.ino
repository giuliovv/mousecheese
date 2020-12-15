#include <ESP8266WiFi.h>
#include <WiFiUdp.h>   
// Set WiFi credentials 

#define UDP_PORT 4210
// UDP
WiFiUDP UDP;
char packet[255];
int reply = 151;


void setup() {
  // Setup serial port
  Serial.begin(9600);
  Serial.println();
   
  // Begin WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
   
  // Connecting to WiFi...
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  // Loop continuously while WiFi is not connected
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    Serial.print(".");
  }
   
  // Connected to WiFi
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  // begin listen to UDP port
  UDP.begin(UDP_PORT);
  Serial.print("Listening on UDP port ");
  Serial.println(UDP_PORT);
   
}

void loop() {
  // If packet received...
  int packetSize = UDP.parsePacket();
  
  if (packetSize) {
 Serial.print("Received packet! Size: ");
    Serial.println(packetSize);
    int len = UDP.read(packet, 255);
    if (len > 0)
    {
      packet[len] = '\0';
    }
    Serial.print("Packet received: ");
    Serial.println(packet);
    // Send return packet
    UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
    UDP.write(reply);
    // UDP.write permette l'invio di dati di tipo int,char o stringhe; non permette invece l'invio di dati float
    UDP.endPacket();
  }
   
}
