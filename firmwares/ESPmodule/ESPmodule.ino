/*
    Project "Weather Station" by MeteoTeam
    Esp module code
*/

#include <ESP8266WiFi.h>
#include <PeriodTimer.h>

// WiFi network Ssid
const char* ssid = "SSID";
// WiFi network Password
const char* password = "PASSWORD";
// Server ThingSpeak
const char* server = "api.thingspeak.com";
// ThingSpeak API KEY
const char* privateKey = "WRITE API KEY";

void setup() {
  Serial.begin(115200);

  connectToWifi();
}

void loop() {
  if (Serial.available()) readData();
}

void connectToWifi()
{
  int attemps = 4;
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && attemps-- > 0) {
    delay(500);
  }
}

void readData()
{
  if (Serial.readStringUntil('\n').toInt() == -131) {
    byte len = Serial.readStringUntil('\n').toInt();
    String data[len];
    for (int i = 0; i < len; i++) data[i] = Serial.readStringUntil('\n');

    sendDataToServer(len, data);
  }
  else Serial.flush();
}

void sendDataToServer(byte len, String data[])
{
  WiFiClient client;
  if (!client.connect(server, 80)) {
    return;
  }

  String url = "/update?key=";
  url += privateKey;

  for (int i = 1; i <= len; i++)
  {
    url += "&field";
    url += String(i);
    url += "=";
    url += data[i - 1].toInt();
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: close\r\n\r\n");
  delay(10);

  client.flush();
  client.stop();
}
