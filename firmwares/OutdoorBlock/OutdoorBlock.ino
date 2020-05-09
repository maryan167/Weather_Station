/*
    Project "Weather Station" by MeteoTeam
    Outdoor Block code
*/

//Library for energy saving
#include <avr/power.h>
#include "LowPower.h"

//PINS
#define POWER_ON 2
#define POWER_OFF 3
#define SLEEP_TIME (long) (20 * 60) / 8   // 20 minutes

#define ACC_PIN A0

#define PMS_TX 0
#define PMS_RX 1  //not used
#define PMS_ON_OFF 4

#define NRF_CE 9
#define NRF_CSN 10

//BME280 libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

//PMS5003 library
#include <SoftwareSerial.h>

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
  float acc_v = -1;
} out_data;

volatile static bool awakened = false;
void wakeUp() {
  if (!awakened) awakened = true;
}

void(* resetFunc) (void) = 0;
void goSleep() {
  if (awakened) 
  {
    awakened = false;
    resetFunc();
  }
  else LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void setRadio()
{
  radio.begin();
  radio.setAutoAck(0);
  radio.setRetries(0, 15);
  radio.setPayloadSize(32);
  radio.setChannel(0x67);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(0x1234567890LL);
  radio.stopListening();
  radio.powerDown();
}

void setup() {
  pinMode(POWER_ON, INPUT_PULLUP);
  pinMode(POWER_OFF, INPUT_PULLUP);
  pinMode(ACC_PIN, INPUT);

  attachInterrupt(0, wakeUp, FALLING);
  attachInterrupt(1, goSleep, FALLING);

  pinMode(PMS_ON_OFF, OUTPUT);
  digitalWrite(PMS_ON_OFF, LOW);

  setRadio();

  clock_prescale_set(clock_div_128);
  delay(1);
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
  delay(1);
  clock_prescale_set(clock_div_1);

  bme.begin(0x76);
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF);
}

void loop() {
  int sleep_count = 0;
  readData();
  sendData();
  if (out_data.acc_v < 1 || out_data.acc_v > 1.4) goSleep();
  delay(1);
  clock_prescale_set(clock_div_128);
  delay(1);
  while (sleep_count++ < SLEEP_TIME) LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  delay(1);
  clock_prescale_set(clock_div_1);
  delay(1);
}

void readData()
{
  out_data.acc_v = (float) (analogRead(ACC_PIN) * 3.3) / 1024;

  bme.takeForcedMeasurement();
  out_data.temp = bme.readTemperature();
  out_data.hum = bme.readHumidity();
  out_data.pres = bme.readPressure() / 100.0F;  //hPa

  digitalWrite(PMS_ON_OFF, HIGH);
  delay(5000);
  SoftwareSerial pmsSerial(PMS_TX, PMS_RX);
  pmsSerial.begin(9600);
  delay(5000);

  if (readPMSdata(&pmsSerial))
  {
    out_data.pm10_standard = data.pm10_standard;
    out_data.pm25_standard = data.pm25_standard;
    out_data.pm100_standard = data.pm100_standard;
  }
  else
  {
    out_data.pm10_standard = -1;
    out_data.pm25_standard = -1;
    out_data.pm100_standard = -1;
  }

  digitalWrite(PMS_ON_OFF, LOW);
  delay(1);
}

void sendData()
{
  radio.powerUp();
  radio.write(&out_data, sizeof(out_data));
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
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)&data, (void *)buffer_u16, 30);

  if (sum != data.checksum) {
    return false;
  }
  // success!
  return true;
}
