/*
    Project "Weather Station" by MeteoTeam
    Central Block code
*/

//#include <Wire.h>

//SETTINGS
#define LED_MIN 50
#define LED_MAX 255
#define BACKLIGHT_MIN 20
#define BACKLIGHT_MAX 255
//#define BRIGHT_CHANGE_LEVEL 100
#define READ_SENSORS_PERIOD 5000

//PINS
#define BTN_PIN 4

#define MHZ_RX 2
#define MHZ_TX 3

#define LED_R 9
#define LED_G 6
#define LED_B 5

#define PHOTOR_PIN A3
#define BACKLIGHT_PIN 10

#include <PeriodTimer.h>

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#include <RTClib.h>
RTC_DS3231 rtc;
DateTime t;

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

#include <MHZ19_uart.h>
MHZ19_uart mhz19;

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8);

PeriodTimer br_pr(5000);
PeriodTimer rs_pr(READ_SENSORS_PERIOD);

//Clock block
uint8_t LT[8] = {0b00111,  0b01111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
uint8_t UB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};
uint8_t RT[8] = {0b11100,  0b11110,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
uint8_t LL[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b01111,  0b00111};
uint8_t LB[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};
uint8_t LR[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11110,  0b11100};
uint8_t UMB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};
uint8_t LMB[8] = {0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};

const char *dayNames[]  = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
};

struct datai
{
  float temp, hum, pres;
  short co2ppm;
} indoor_data;

struct datao
{
  float temp, hum, pres;
  bool check;
  short brightness;
} outdoor_data;

void setup()
{
  Serial.begin(9600);
  delay(20);
  pinMode(PHOTOR_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);
  radio.begin();
  radio.setPayloadSize(32);
  radio.setChannel(7);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openReadingPipe(1, 0x1234567890LL);
  radio.powerUp();
  radio.startListening();
  bme.begin(0x76);
  lcd.init();
  lcd.backlight();
  loadClock();
  readSensors();
  checkBrightness();
}

byte mode = 1;
bool isMChanged = false;

void loop()
{
  checkBrightness();
  checkButton();
  outdoor_data.check = 0;
  readSensors();
  if (mode == 1)
  {
    t = rtc.now();
    drawClock();
    drawDate();
    drawMainScreenData();
  }
  else if (mode == 2)
  {
    drawIndoorData();
  }
  else if (mode == 3)
  {
    drawOutdoorData();
  }

  isMChanged = false;
}

byte LED_BRIGHT;
void checkBrightness()
{
  if (br_pr.isReady())
  {
    if (digitalRead(PHOTOR_PIN))
    {
      analogWrite(BACKLIGHT_PIN, BACKLIGHT_MIN);
      LED_BRIGHT = LED_MIN;
    }
    else
    {
      analogWrite(BACKLIGHT_PIN, BACKLIGHT_MAX);
      LED_BRIGHT = LED_MAX;
    }
    checkCO2Led();
  }
}

void checkCO2Led()
{
  if (indoor_data.co2ppm < 800) setLED(2);
  else if (indoor_data.co2ppm < 1200) setLED(3);
  else if (indoor_data.co2ppm >= 1200) setLED(1);
}

void setLED(byte color) {
  analogWrite(LED_R, 0);
  analogWrite(LED_G, 0);
  analogWrite(LED_B, 0);
  switch (color) {
    case 0:
      break;
    case 1: analogWrite(LED_R, LED_BRIGHT);
      break;
    case 2: analogWrite(LED_G, LED_BRIGHT);
      break;
    case 3: analogWrite(LED_B, LED_BRIGHT);
      break;
  }
}

void readSensors()
{
  if (rs_pr.isReady())
  {
    indoor_data.temp = bme.readTemperature();
    indoor_data.hum = bme.readHumidity();
    indoor_data.pres = bme.readPressure();
    indoor_data.co2ppm = mhz19.getPPM();
  }

  if (radio.available())
  {
    radio.read(&outdoor_data, sizeof(outdoor_data));
  }
}

void drawMainScreenData()
{
  lcd.setCursor(0, 2);
  lcd.print(indoor_data.temp, 1);
  lcd.print('\337');
  //lcd.print("C  ");
  lcd.print("  ");
  lcd.print(outdoor_data.temp, 1);
  lcd.print('\337');
  //lcd.print("C  ");
  lcd.print(" ");
  lcd.print(outdoor_data.check);
  lcd.print("  CO2");
  lcd.setCursor(2, 3);
  lcd.print(indoor_data.hum, 0);
  lcd.print("%   ");
  lcd.print(outdoor_data.hum, 0);
  lcd.print("%  ");
  if (indoor_data.co2ppm < 1000) lcd.print(" ");
  lcd.print(String(indoor_data.co2ppm) + "ppm");
}

void drawIndoorData()
{
  lcd.setCursor(6, 0);
  lcd.print("INDOOR");
  lcd.setCursor(0, 1);
  lcd.print("T: ");
  lcd.print(indoor_data.temp, 1);
  lcd.print('\337');
  lcd.print("C");
  lcd.setCursor(0, 2);
  lcd.print("H: ");
  lcd.print(indoor_data.hum, 0);
  lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("P: ");
  lcd.print(indoor_data.pres / 133.33, 0);
  lcd.print("mmh");
}

void drawOutdoorData()
{
  lcd.setCursor(6, 0);
  lcd.print("OUTDOOR");
  lcd.setCursor(0, 1);
  lcd.print("T: ");
  lcd.print(outdoor_data.temp, 1);
  lcd.print('\337');
  lcd.print("C");
  lcd.setCursor(11, 1);
  lcd.print("B: ");
  lcd.print(outdoor_data.brightness, 1);
  lcd.print("   ");
  lcd.setCursor(0, 2);
  lcd.print("H: ");
  lcd.print(outdoor_data.hum, 0);
  lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("P: ");
  lcd.print(outdoor_data.pres, 0);
  lcd.print("mmh");
}

bool isClicked = false;
void checkButton()
{
  unsigned long btn_click_time = millis();
  if (digitalRead(BTN_PIN) == HIGH)
  {
    while (digitalRead(BTN_PIN) && !isClicked)
    {
      if (short(millis() - btn_click_time) > 0) {
        isClicked = true;
        if (mode + 1 == 4) mode = 1;
        else mode++;
        isMChanged = true;
        clearLCD();
        return;
      }
    }
  }
  else isClicked = false;
}

void clearLCD()
{
  lcd.setCursor(0, 0);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
  delay(100);
}
