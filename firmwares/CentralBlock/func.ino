uint8_t LT[8] = {0b00111,  0b01111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
uint8_t UB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000};
uint8_t RT[8] = {0b11100,  0b11110,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111};
uint8_t LL[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b01111,  0b00111};
uint8_t LB[8] = {0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};
uint8_t LR[8] = {0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11111,  0b11110,  0b11100};
uint8_t UMB[8] = {0b11111,  0b11111,  0b11111,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111};
uint8_t LMB[8] = {0b11111,  0b00000,  0b00000,  0b00000,  0b00000,  0b11111,  0b11111,  0b11111};

void loadClock() {
  lcd.createChar(0, LT);
  lcd.createChar(1, UB);
  lcd.createChar(2, RT);
  lcd.createChar(3, LL);
  lcd.createChar(4, LB);
  lcd.createChar(5, LR);
  lcd.createChar(6, UMB);
  lcd.createChar(7, LMB);
}

void drawDig(byte dig, byte x, byte y) {
  switch (dig) {
    case 0:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(4);
      lcd.write(5);
      break;
    case 1:
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(5);
      break;
    case 2:
      lcd.setCursor(x, y);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(7);
      break;
    case 3:
      lcd.setCursor(x, y);
      lcd.write(6);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
      break;
    case 4:
      lcd.setCursor(x, y);
      lcd.write(3);
      lcd.write(4);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(32);
      lcd.write(5);
      break;
    case 5:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(7);
      lcd.write(7);
      lcd.write(5);
      break;
    case 6:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(6);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
      break;
    case 7:
      lcd.setCursor(x, y);
      lcd.write(1);
      lcd.write(1);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(0);
      lcd.write(32);
      break;
    case 8:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(3);
      lcd.write(7);
      lcd.write(5);
      break;
    case 9:
      lcd.setCursor(x, y);
      lcd.write(0);
      lcd.write(6);
      lcd.write(2);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(4);
      lcd.write(5);
      break;
    case 10:
      lcd.setCursor(x, y);
      lcd.write(32);
      lcd.write(32);
      lcd.write(32);
      lcd.setCursor(x, y + 1);
      lcd.write(32);
      lcd.write(32);
      lcd.write(32);
      break;
  }
}

void drawDot()
{
  static unsigned long dot_t = 0;
  const short dot_delay = 700;

  if (isMChanged) dot_t = millis();
  if (short(millis() - dot_t) > 0)
  {
    dot_t = millis() + 2 * dot_delay;
    lcd.setCursor(7, 0);
    lcd.write(165);
    lcd.setCursor(7, 1);
    lcd.write(165);
  }
  if (short(millis() - dot_t) > -dot_delay && short(millis() - dot_t) < -0.9 * dot_delay)
  {
    lcd.setCursor(7, 0);
    lcd.print(" ");
    lcd.setCursor(7, 1);
    lcd.print(" ");
  }
}

void drawTime(byte h, byte m)
{
  if (h % 10 == 1)
  {
    lcd.setCursor(0, 0);
    lcd.print("  ");
    lcd.setCursor(0, 1);
    lcd.print("  ");
    lcd.setCursor(4, 0);
    lcd.print(" ");
    lcd.setCursor(4, 1);
    lcd.print(" ");

    if (h / 10 == 1) drawDig(1, 2, 0);
    else drawDig(h / 10, 1, 0);

    drawDig(1, 5, 0);
  }
  else
  {
    lcd.setCursor(3, 0);
    lcd.print(" ");
    lcd.setCursor(3, 1);
    lcd.print(" ");
    if (h / 10 == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(" ");
      drawDig(1, 1, 0);
    }
    else drawDig(h / 10, 0, 0);
    drawDig(h % 10, 4, 0);
  }
  if (m / 10 == 1)
  {
    drawDig(1, 8, 0);
    lcd.setCursor(10, 0);
    lcd.print(" ");
    lcd.setCursor(10, 1);
    lcd.print(" ");
    if (m % 10 == 1)
    {
      drawDig(1, 11, 0);
      lcd.setCursor(13, 0);
      lcd.print(" ");
      lcd.setCursor(13, 1);
      lcd.print(" ");
    }
    else
    {
      drawDig(m % 10, 11, 0);
      lcd.setCursor(14, 0);
      lcd.print(" ");
      lcd.setCursor(14, 1);
      lcd.print(" ");
    }
  }
  else
  {
    drawDig(m / 10, 8, 0);
    lcd.setCursor(11, 0);
    lcd.print(" ");
    lcd.setCursor(11, 1);
    lcd.print(" ");
    if (m % 10 == 1)
    {
      lcd.setCursor(14, 0);
      lcd.print(" ");
      lcd.setCursor(14, 1);
      lcd.print(" ");
      drawDig(1, 12, 0);
    }
    else drawDig(m % 10, 12, 0);
  }
}

void drawClock()
{
  static byte last_min;
  drawDot();
  t = rtc.now();
  byte h = t.hour();
  byte m = t.minute();
  if (last_min != m || isMChanged)
  {
    drawTime(h, m);
    last_min = m;
  }
}

const char *dayNames[]  = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat",
};

void drawDate()
{
  static byte last_day;
  if (last_day != t.day() || isMChanged)
  {
    last_day = t.day();
    lcd.setCursor(15, 0);
    if (t.day() < 10) lcd.print("0");
    lcd.print(t.day());
    lcd.print("/");
    if (t.month() < 10) lcd.print("0");
    lcd.print(t.month());
    lcd.setCursor(17, 1);
    lcd.print(dayNames[t.dayOfTheWeek()]);
  }
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

void checkCO2Led()
{
  if (in_data.co2ppm < 800) setLED(2);
  else if (in_data.co2ppm < 1200) setLED(3);
  else if (in_data.co2ppm >= 1200) setLED(1);
}
