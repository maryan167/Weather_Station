/*
    Project "Weather Station" by MeteoTeam
    Outdoor Block code
*/

//PINS
#define POWER_OFF 0 // pin D2
#define POWER_ON 7

#define PMS_TX 3
#define PMS_RX 4
#define PMS_SLEEP 5

#define NRF_CE 9
#define NRF_CSN 10

//Library for energy saving
#include "LowPower.h"

//BME280 libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

//PMS5003 library
#include <SoftwareSerial.h>
SoftwareSerial pmsSerial(PMS_TX, PMS_RX);

//nRF24L01 libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(NRF_CE, NRF_CSN);

struct pms5003data {
  uint16_t framelen;
  uint16_t pm10_standard, pm25_standard, pm100_standard;
  uint16_t pm10_env, pm25_env, pm100_env;
  uint16_t particles_03um, particles_05um, particles_10um, particles_25um, particles_50um, particles_100um;
  uint16_t unused;
  uint16_t checksum;
} data;

struct datao
{
  float temp = 0, hum = 0, pres = 0;
  unsigned int pm10_standard = 0, pm25_standard = 0, pm100_standard = 0;
} out_data;

void powerOff() {digitalWrite(POWER_ON, LOW);}

void setup() {
  pinMode(POWER_ON, OUTPUT);
  digitalWrite(POWER_ON, HIGH);
  delay(10);

  attachInterrupt(POWER_OFF, powerOff, RISING);

  pinMode(PMS_SLEEP, OUTPUT);
  digitalWrite(PMS_SLEEP, HIGH);
  
  radio.begin();
  radio.setAutoAck(0);
  radio.setRetries(0,15);
  radio.setPayloadSize(32);
  radio.setChannel(0x67);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(0x1234567890LL);
  radio.powerUp();
  radio.stopListening();
  
  bme.begin(0x76);
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF);

  pmsSerial.begin(9600);
}

int sleep_time = 10;
void loop() {
  byte sleep_count = 0;
  readData();
  sendData();
  while (sleep_count++ < sleep_time) LowPower.powerDown(SLEEP_2S, ADC_OFF, BOD_OFF);
  delay(100);
}

void readData()
{
  bme.takeForcedMeasurement();
  out_data.temp = bme.readTemperature();
  out_data.hum = bme.readHumidity();
  out_data.pres = bme.readPressure() / 100.0F;  //hPa

  digitalWrite(PMS_SLEEP, HIGH);
  delay(10000);
  if (readPMSdata(&pmsSerial)) 
  {
    out_data.pm10_standard = data.pm10_standard;
    out_data.pm25_standard = data.pm25_standard;
    out_data.pm100_standard = data.pm100_standard;
  }
  else 
  {
    out_data.pm10_standard = 6666;
    out_data.pm25_standard = 6666;
    out_data.pm100_standard = 6666;
  }
  digitalWrite(PMS_SLEEP, LOW);
}

void sendData()
{
  radio.powerUp();
  delay(1000);
  radio.write(&out_data, sizeof(out_data));
  delay(1000);
  radio.powerDown();
}

boolean readPMSdata(Stream *s) {
  if (! s->available()) {
    return false;
  }
  
  // Read a byte at a time until we get to the special '0x42' start-byte
  if (s->peek() != 0x42) {
    s->read();
    return false;
  }

  // Now read all 32 bytes
  if (s->available() < 32) {
    return false;
  }
    
  uint8_t buffer[32];    
  uint16_t sum = 0;
  s->readBytes(buffer, 32);

  // get checksum ready
  for (uint8_t i=0; i<30; i++) {
    sum += buffer[i];
  }
  
  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i=0; i<15; i++) {
    buffer_u16[i] = buffer[2 + i*2 + 1];
    buffer_u16[i] += (buffer[2 + i*2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);

  if (sum != data.checksum) {
    return false;
  }
  // success!
  return true;
}
