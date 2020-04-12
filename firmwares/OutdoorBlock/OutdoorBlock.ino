/*
    Project "Weather Station" by MeteoTeam
    Outdoor Block code
*/

//Library for energy saving
#include "LowPower.h"

//nRF24L01 libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10);

struct datao
{
  float temp, hum, pres;
  short brightness;
} outdoor_data;

void setup() {
  pinMode(A3, INPUT);
  radio.begin();
  radio.setAutoAck(1);
  radio.setRetries(1,5);
  radio.setPayloadSize(32);
  radio.setChannel(7);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(0x1234567890LL);
  radio.powerUp();
  radio.stopListening();
  outdoor_data.temp = 9.7;
  outdoor_data.hum = 29;
  outdoor_data.pres = 749;
}

byte sleep_count;
void loop() {
  sleep_count = 0;
  readData();
  sendData();
  while (sleep_count < 60)
  {
    LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
    ++sleep_count;
  }
}

void readData()
{
  outdoor_data.pres = analogRead(A2);
  outdoor_data.brightness = analogRead(A3);
}

void sendData()
{
  radio.powerUp();
  radio.write(&outdoor_data, sizeof(outdoor_data));
  radio.powerDown();
}
