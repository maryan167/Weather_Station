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
#define BRIGHTNESS_CHECK_PERIOD 2000
#define READ_SENSORS_PERIOD 5000

//PINS
#define BTN_PIN 4

#define MHZ_RX 2
#define MHZ_TX 3

#define CE 7
#define CSN 8

#define LED_R 9
#define LED_G 6
#define LED_B 5

#define PHOTOR_PIN A3
#define BACKLIGHT_PIN 10

#define ESP_TX 14
#define ESP_RX 15

//Timer library
#include <PeriodTimer.h>

//Lcd Dispaly library
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

//DS3231 library
#include <RTClib.h>
RTC_DS3231 rtc;
DateTime t;

//BME280 libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

//MHZ19B library
#include <MHZ19_uart.h>
MHZ19_uart mhz19;

//nRf24L01 libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(CE, CSN);

//Esp8266 library
#include <SoftwareSerial.h>
SoftwareSerial esp8266(ESP_TX, ESP_RX);

PeriodTimer br_pr(BRIGHTNESS_CHECK_PERIOD);
PeriodTimer rs_pr(READ_SENSORS_PERIOD);
PeriodTimer data_send_t((long) 2 * 60 * 1000);

struct datai
{
  float temp;
  short hum, pres, co2ppm;
} in_data;

struct datao
{
  float temp, hum, pres;
  short brightness;
} out_data;

void setup()
{
  pinMode(PHOTOR_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  esp8266.begin(115200);
  radio.begin();
  radio.setPayloadSize(32);
  radio.setChannel(7);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openReadingPipe(1, 0x1234567890LL);
  radio.powerUp();
  radio.startListening();
  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);
  bme.begin(0x76);
  lcd.init();
  loadClock();
  lcd.backlight();
}

byte mode = 1;
bool isMChanged = false;
void loop()
{
  checkBrightness();
  checkButton();
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

  if (data_send_t.isReady()) sendDataToCloud();
}

void readSensors()
{
  if (rs_pr.isReady())
  {
    in_data.temp = bme.readTemperature();
    in_data.hum = short(bme.readHumidity() + 0.5);
    in_data.pres = short((float) bme.readPressure() * 0.0075006);
    in_data.co2ppm = mhz19.getPPM();
  }

  if (radio.available())
  {
    radio.read(&out_data, sizeof(out_data));
  }
}

void drawMainScreenData()
{
  lcd.setCursor(0, 2);
  lcd.print(in_data.temp, 1);
  lcd.print('\337');
  lcd.print("  ");
  lcd.setCursor(2, 3);
  lcd.print(in_data.hum);
  lcd.print("% ");

  lcd.setCursor(7, 2);
  lcd.print(out_data.temp, 1);
  lcd.print('\337');
  lcd.print("  ");
  lcd.setCursor(8, 3);
  lcd.print(out_data.hum, 0);
  lcd.print("% ");

  lcd.setCursor(13, 2);
  if (short((float)in_data.co2ppm * 0.4 + 400) < 1000) lcd.print(" ");
  lcd.print(short((float)in_data.co2ppm * 0.4 + 400));

  lcd.setCursor(13, 3);
  if (in_data.co2ppm < 1000) lcd.print(" ");
  lcd.print(String(in_data.co2ppm) + "ppm");
}

void drawIndoorData()
{
  lcd.setCursor(6, 0);
  lcd.print("INDOOR");
  lcd.setCursor(0, 1);
  lcd.print("T: ");
  lcd.print(in_data.temp, 1);
  lcd.print('\337');
  lcd.print("C");
  lcd.setCursor(0, 2);
  lcd.print("H: ");
  lcd.print(in_data.hum);
  lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("P: ");
  lcd.print(in_data.pres / 133.33, 0);
  lcd.print("mmh");
}

void drawOutdoorData()
{
  lcd.setCursor(6, 0);
  lcd.print("OUTDOOR");
  lcd.setCursor(0, 1);
  lcd.print("T: ");
  lcd.print(out_data.temp, 1);
  lcd.print('\337');
  lcd.print("C");
  lcd.setCursor(11, 1);
  lcd.print("B: ");
  lcd.print(out_data.brightness, 1);
  lcd.print("   ");
  lcd.setCursor(0, 2);
  lcd.print("H: ");
  lcd.print(out_data.hum, 0);
  lcd.print("%");
  lcd.setCursor(0, 3);
  lcd.print("P: ");
  lcd.print(out_data.pres, 0);
  lcd.print("mmh");
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

void checkButton()
{
  static bool isClicked = false;
  unsigned long btn_click_time = millis();
  if (digitalRead(BTN_PIN) == HIGH)
  {
    while (digitalRead(BTN_PIN) && !isClicked)
    {
      if (short(millis() - btn_click_time) > 0) {
        isClicked = true;
        if (mode + 1 > 3) mode = 1;
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
  //delay(100);
}

void sendDataToCloud()
{
  const byte len = 4;
  String data[len];
  data[0] = String(in_data.temp);
  data[1] = String(in_data.hum);
  data[2] = String(in_data.pres);
  data[3] = String(in_data.co2ppm);
  //  data[4] = String("");
  //  data[5] = String("");
  //  data[6] = String("");
  //  data[7] = String("");
  esp8266.println(String(-131));
  esp8266.println(String(len));
  for (int i = 0; i < len; i++) esp8266.println(data[i]);
}
