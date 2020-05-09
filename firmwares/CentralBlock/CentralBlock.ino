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

#define NRF_CE 7
#define NRF_CSN 8

#define LED_R 9
#define LED_G 6
#define LED_B 5

#define PHOTOR_PIN A3
#define BACKLIGHT_PIN 10

#define ESP_TX 14
#define ESP_RX 15
#define ESP_CONTROL 16

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
#define SEALEVELPRESSURE_HPA (1013.25)  //bme.readAltitude(SEALEVELPRESSURE_HPA)
Adafruit_BME280 bme;

//MHZ19B library
#include <MHZ19_uart.h>
MHZ19_uart mhz19;

//nRf24L01 libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(NRF_CE, NRF_CSN);

//Esp8266 library
#include <SoftwareSerial.h>
SoftwareSerial esp8266(ESP_TX, ESP_RX);

PeriodTimer br_pr(BRIGHTNESS_CHECK_PERIOD);
PeriodTimer rs_pr(READ_SENSORS_PERIOD);
PeriodTimer wcc_pr((long) 10 * 60 * 1000);
PeriodTimer data_send_t((long) 2 * 60 * 1000);

uint32_t pressure_array[6];
byte time_array[6];
short wcc = 0;

struct datai
{
  float temp = 0, hum = 0, pres = 0;
  short co2ppm = 0;
} in_prev, in_data;

struct datao
{
  float temp = 0, hum = 0, pres = 0;
  unsigned int pm10_s = 0, pm25_s = 0, pm100_s = 0;
  float acc_v = -1;
} out_prev, out_data;

void setup()
{
  pinMode(PHOTOR_PIN, INPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  pinMode(ESP_CONTROL, OUTPUT);
  esp8266.begin(115200);

  radio.begin();
  radio.setAutoAck(0);
  radio.setRetries(0, 15);
  radio.setPayloadSize(32);
  radio.setChannel(0x67);
  radio.setDataRate(RF24_1MBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.openReadingPipe(1, 0x1234567890LL);
  radio.powerUp();
  radio.startListening();

  mhz19.begin(MHZ_TX, MHZ_RX);
  mhz19.setAutoCalibration(false);

  bme.begin(0x76);
  bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                  Adafruit_BME280::SAMPLING_X2,  // temperature
                  Adafruit_BME280::SAMPLING_X16, // pressure
                  Adafruit_BME280::SAMPLING_X1,  // humidity
                  Adafruit_BME280::FILTER_X16,
                  Adafruit_BME280::STANDBY_MS_0_5);

  uint32_t Press = bme.readPressure();
  for (byte i = 0; i < 6; i++) {
    pressure_array[i] = Press;
    time_array[i] = i;
  }
  
  lcd.init();
  loadClock();
  lcd.backlight();
}

byte mode = 1;
bool isMChanged = true;
void loop()
{
  checkBrightness();
  checkButton();
  readSensors();
  if(wcc_pr.isReady()) checkWeather();

  if (mode == 1) drawMainScreenData();
  else if (mode == 2) drawIndoorData();
  else if (mode == 3) drawOutdoorData();

  isMChanged = false;

  if (data_send_t.isReady()) sendDataToCloud(1);
  else sendDataToCloud(0);
}



bool in_prev_status = false;
bool out_prev_status = false;
void readSensors()
{
  static byte p = 0;
  byte in_prev_period = 10;
  if (rs_pr.isReady())
  {
    if (p-- == 0)
    {
      in_prev_status = true;
      p = in_prev_period;
      in_prev.temp = in_data.temp;
      in_prev.hum = in_data.hum;
      in_prev.pres = in_data.pres;
      in_prev.co2ppm = in_data.co2ppm;
    }
    else in_prev_status = false;

    //bme.takeForcedMeasurement();
    in_data.temp = bme.readTemperature();
    in_data.hum = bme.readHumidity();
    in_data.pres = (float) bme.readPressure() * 0.0075006;
    in_data.co2ppm = mhz19.getPPM();
  }

  if (radio.available())
  {
    out_prev_status = true;
    out_prev.temp = out_data.temp;
    out_prev.hum = out_data.hum;
    out_prev.pres = out_data.pres;
    radio.read(&out_data, sizeof(out_data));
    out_data.pres = (float) out_data.pres * 0.75006;
  }
  else out_prev_status = false;
}

void drawMainScreenData()
{
  if (isMChanged)
  {
    loadClock();


    lcd.setCursor(0, 2);
    lcd.print("I:");

    lcd.setCursor(7, 2);
    lcd.print('\337');

    lcd.setCursor(11, 2);
    lcd.print("%");

    lcd.setCursor(17, 2);
    lcd.print("ppm");


    lcd.setCursor(0, 3);
    lcd.print("O:");

    lcd.setCursor(7, 3);
    lcd.print('\337');

    lcd.setCursor(11, 3);
    lcd.print("%");

    lcd.setCursor(13, 3);
    lcd.print("wc:");
    lcd.setCursor(19, 3);
    lcd.print("%");
  }

  drawClock();
  drawDate();

  lcd.setCursor(2, 2);
  if (in_data.temp > -10) lcd.print(" ");
  if (in_data.temp >= 0 && in_data.temp < 10) lcd.print(" ");
  lcd.print(in_data.temp, 1);

  lcd.setCursor(9, 2);
  if (in_data.hum < 10) lcd.print(" ");
  lcd.print(constrain(in_data.hum, 0, 99), 0);

  lcd.setCursor(13, 2);
  if (in_data.co2ppm < 1000) lcd.print(" ");
  lcd.print(in_data.co2ppm);


  lcd.setCursor(2, 3);
  if (out_data.temp > -10) lcd.print(" ");
  if (out_data.temp >= 0 && out_data.temp < 10) lcd.print(" ");
  lcd.print(out_data.temp, 1);

  lcd.setCursor(9, 3);
  if (out_data.hum < 10) lcd.print(" ");
  lcd.print(constrain(out_data.hum, 0, 99), 0);


  //  lcd.setCursor(13, 2);
  //  if (short((float)in_data.co2ppm * 0.32 + 400) < 1000) lcd.print(" ");
  //  lcd.print(short((float)in_data.co2ppm * 0.32 + 400));

  lcd.setCursor(16, 3);
  if(abs(wcc) == 0) lcd.print(" ");
  if(abs(wcc) < 10) lcd.print(" ");
  if(wcc > 0) lcd.print("+");
  lcd.print(wcc);
}


void checkWeather() 
{
  long pressure = 0;
  for (byte i = 0; i < 10; i++) {
    pressure += bme.readPressure();
  }
  pressure /= 10;

  unsigned long sumX, sumY, sumX2, sumXY;
  for (byte i = 0; i < 5; i++) {
      pressure_array[i] = pressure_array[i + 1];
    }
    pressure_array[5] = pressure;

    sumX = 0;
    sumY = 0;
    sumX2 = 0;
    sumXY = 0;
    for (int i = 0; i < 6; i++) {
      sumX += time_array[i];
      sumY += (long)pressure_array[i];
      sumX2 += time_array[i] * time_array[i];
      sumXY += (long)time_array[i] * pressure_array[i];
    }
    float a = 0;
    a = (long)6 * sumXY;
    a = a - (long)sumX * sumY;
    a = (float)a / (6 * sumX2 - sumX * sumX);
    float delta = a * 6;

    wcc = map(delta, -250, 250, 100, -100);
}

byte term[] = {
  B00100,
  B01110,
  B01010,
  B01110,
  B01010,
  B10101,
  B10001,
  B01110
};

byte drop[] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B10111,
  B10001,
  B01110,
  B00000
};

byte pres[] = {
  B00000,
  B00000,
  B01110,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000
};

uint8_t arrow_up[] = {0b00100, 0b01110, 0b10101, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000};
uint8_t arrow_down[] = {0b00100, 0b00100, 0b00100, 0b00100, 0b10101, 0b01110, 0b00100, 0b00000};

void loadIndoorSymbols()
{
  lcd.createChar(0, term);
  lcd.createChar(1, drop);
  lcd.createChar(2, pres);
  lcd.createChar(3, arrow_up);
  lcd.createChar(4, arrow_down);
}

void drawIndoorData()
{
  if (isMChanged)
  {
    loadIndoorSymbols();

    lcd.setCursor(14, 0);
    lcd.print("Indoor");

    lcd.setCursor(0, 0);
    lcd.write(0);
    lcd.print(":");
    lcd.setCursor(7, 0);
    lcd.print('\337');
    lcd.print("(");
    lcd.setCursor(10, 0);
    lcd.print(")");

    lcd.setCursor(0, 1);
    lcd.write(1);
    lcd.print(":");
    lcd.setCursor(5, 1);
    lcd.print("%(");
    lcd.setCursor(8, 1);
    lcd.print(")");

    lcd.setCursor(0, 2);
    lcd.write(2);
    lcd.print(":");
    lcd.setCursor(6, 2);
    lcd.print("mmh(");
    lcd.setCursor(11, 2);
    lcd.print(")");

    lcd.setCursor(0, 3);
    lcd.print("Co2:");
    lcd.setCursor(8, 3);
    lcd.print("ppm(");
    lcd.setCursor(13, 3);
    lcd.print(")");
  }

  //Temperature
  lcd.setCursor(2, 0);
  if (in_data.temp > -10) lcd.print(" ");
  if (in_data.temp >= 0 && in_data.temp < 10) lcd.print(" ");
  lcd.print(in_data.temp, 1);
  lcd.setCursor(9, 0);
  if (in_data.temp - in_prev.temp > 0.1 && (in_prev_status || isMChanged)) lcd.write(3);
  else if (in_prev.temp - in_data.temp > 0.1 && in_prev_status) lcd.write(4);
  else if (in_prev_status || isMChanged) lcd.print(".");

  //Humidity
  lcd.setCursor(2, 1);
  if (in_data.hum != 100) lcd.print(" ");
  if (in_data.hum < 10) lcd.print(" ");
  lcd.print(in_data.hum, 0);
  lcd.setCursor(7, 1);
  if (in_data.hum - in_prev.hum > 0.5 && (in_prev_status || isMChanged)) lcd.write(3);
  else if (in_prev.hum - in_data.hum > 0.5 && (in_prev_status || isMChanged)) lcd.write(4);
  else if (in_prev_status || isMChanged) lcd.print(".");

  //Pressure
  lcd.setCursor(2, 2);
  if (in_data.pres < 1000) lcd.print(" ");
  lcd.print(in_data.pres, 0);
  lcd.setCursor(10, 2);
  if (in_data.pres - in_prev.pres > 0.5 && (in_prev_status || isMChanged)) lcd.write(3);
  else if (in_prev.pres - in_data.pres > 0.5 && (in_prev_status || isMChanged)) lcd.write(4);
  else if (in_prev_status || isMChanged) lcd.print(".");

  //Co2
  lcd.setCursor(4, 3);
  if (in_data.co2ppm < 1000) lcd.print(" ");
  lcd.print(in_data.co2ppm);
  lcd.setCursor(12, 3);
  if (in_data.co2ppm - in_prev.co2ppm > 50 && (in_prev_status || isMChanged)) lcd.write(3);
  else if (in_prev.co2ppm - in_data.co2ppm > 50 && (in_prev_status || isMChanged)) lcd.write(4);
  else if (in_prev_status || isMChanged) lcd.print(".");
}

byte sun_symb[] = {
  B00000,
  B01110,
  B11111,
  B11111,
  B11111,
  B01110,
  B00000,
  B00000
};

byte cloud_symb[] = {
  B00000,
  B01110,
  B11111,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000
};

byte rain_symb[] = {
  B01110,
  B11111,
  B11111,
  B00000,
  B01010,
  B00000,
  B01010,
  B00000
};

byte btr_symb[] = {
  B01110,
  B11011,
  B10001,
  B10101,
  B11111,
  B10101,
  B10001,
  B11111
};

void loadOutdoorSymbols()
{
  lcd.createChar(0, term);
  lcd.createChar(1, drop);
  lcd.createChar(2, pres);
  lcd.createChar(3, btr_symb);
}

void drawOutdoorData()
{
  if (isMChanged)
  {
    loadOutdoorSymbols();

    lcd.setCursor(13, 0);
    lcd.print("Outdoor");

    lcd.setCursor(0, 0);
    lcd.write(0);
    lcd.print(":");
    lcd.setCursor(7, 0);
    lcd.print('\337');
    lcd.print("(");
    lcd.setCursor(10, 0);
    lcd.print(")");

    lcd.setCursor(0, 1);
    lcd.write(1);
    lcd.print(":");
    lcd.setCursor(5, 1);
    lcd.print("%(");
    lcd.setCursor(8, 1);
    lcd.print(")");

    lcd.setCursor(14, 1);
    lcd.write(3);
    lcd.setCursor(19, 1);
    lcd.print("v");

    lcd.setCursor(0, 2);
    lcd.write(2);
    lcd.print(":");
    lcd.setCursor(6, 2);
    lcd.print("mmh(");
    lcd.setCursor(11, 2);
    lcd.print(")");
    lcd.setCursor(14, 2);
    lcd.print("1:");

    lcd.setCursor(0, 3);
    lcd.print("Pm: 10:");
    lcd.setCursor(12, 3);
    lcd.print("2.5:");
  }

  //Temperature
  lcd.setCursor(2, 0);
  if (out_data.temp > -10) lcd.print(" ");
  if (out_data.temp >= 0 && out_data.temp < 10) lcd.print(" ");
  lcd.print(out_data.temp, 1);
  lcd.setCursor(9, 0);
  if (out_data.temp - out_prev.temp > 0.1 && (out_prev_status || isMChanged)) lcd.write(3);
  else if (out_prev.temp - out_data.temp > 0.1 && (out_prev_status || isMChanged)) lcd.write(4);
  else if (out_prev_status || isMChanged) lcd.print(".");

  //Humidity
  lcd.setCursor(2, 1);
  if (out_data.hum != 100) lcd.print(" ");
  if (out_data.hum < 10) lcd.print(" ");
  lcd.print(out_data.hum, 0);
  lcd.setCursor(7, 1);
  if (out_data.hum - out_prev.hum > 0.5 && (out_prev_status || isMChanged)) lcd.write(3);
  else if (out_prev.hum - out_data.hum > 0.5 && (out_prev_status || isMChanged)) lcd.write(4);
  else if (out_prev_status || isMChanged) lcd.print(".");

  //Outdoor battery voltage
  lcd.setCursor(15, 1);
  if(out_data.acc_v == -1) lcd.print(" -- ");
  else lcd.print(out_data.acc_v, 2);

  //Pressure
  lcd.setCursor(2, 2);
  if (out_data.pres == 0) lcd.print("  ");
  if (out_data.pres < 1000) lcd.print(" ");
  lcd.print(out_data.pres, 0);
  lcd.setCursor(10, 2);
  if (out_data.pres - out_prev.pres > 0.5 && (out_prev_status || isMChanged)) lcd.write(3);
  else if (out_prev.pres - out_data.pres > 0.5 && (out_prev_status || isMChanged)) lcd.write(4);
  else if (out_prev_status || isMChanged) lcd.print(".");

  //Pm1
  lcd.setCursor(16, 2);
  lcd.print(out_data.pm10_s);
  if (out_data.pm10_s < 10) lcd.print("   ");
  else if (out_data.pm10_s < 100) lcd.print("  ");
  else if (out_data.pm10_s < 1000) lcd.print(" ");

  //Pm10
  lcd.setCursor(7, 3);
  lcd.print(out_data.pm100_s);
  if (out_data.pm100_s < 10) lcd.print("   ");
  else if (out_data.pm100_s < 100) lcd.print("  ");
  else if (out_data.pm100_s < 1000) lcd.print(" ");

  //Pm2.5
  lcd.setCursor(16, 3);
  lcd.print(out_data.pm25_s);
  if (out_data.pm25_s < 10) lcd.print("   ");
  else if (out_data.pm25_s < 100) lcd.print("  ");
  else if (out_data.pm25_s < 1000) lcd.print(" ");
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
}

void sendDataToCloud(bool s)
{
  static unsigned int send_step = 0;
  if(s == 1) send_step = 1;
  
  int last_step = 20;
  if (send_step == 1)
  {
    send_step++;
    digitalWrite(ESP_CONTROL, HIGH);
  }
  else if (send_step >= 2 && send_step < last_step)
  {
    send_step++;
    digitalWrite(ESP_CONTROL, LOW);
  }
  else if (send_step == last_step)
  {
    send_step = 0;
    
    const byte len = 8;
    String data[len];
    data[0] = String(in_data.temp);
    data[1] = String(in_data.hum);
    data[2] = String(in_data.pres);
    data[3] = String(in_data.co2ppm);
    data[4] = String(out_data.temp);
    data[5] = String(out_data.hum);
    data[6] = String(out_data.pm25_s);
    data[7] = String(out_data.pm100_s);
    esp8266.println(String(-131));
    esp8266.println(String(len));
    for (int i = 0; i < len; i++) esp8266.println(data[i]);
  }
}
