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

unsigned long dot_t = 0;
short dot_delay = 700;
void drawDot()
{
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

void drawTime()
{
  byte h = t.hour();
  byte m = t.minute();
  if (h % 10 == 1)
  {
    lcd.setCursor(4, 0);
    lcd.print(" ");
    lcd.setCursor(4, 1);
    lcd.print(" ");
    if (h / 10 == 1)
    {
      lcd.setCursor(0, 0);
      lcd.print("  ");
      lcd.setCursor(0, 1);
      lcd.print("  ");
      drawDig(1, 2, 0);
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print(" ");
      lcd.setCursor(0, 1);
      lcd.print(" ");
      drawDig(h / 10, 1, 0);
    }
    drawDig(1, 5, 0);
  }
  else
  {
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
      lcd.setCursor(13, 0);
      lcd.print("  ");
      lcd.setCursor(13, 1);
      lcd.print("  ");
      drawDig(1, 11, 0);
    }
    else
    {
      lcd.setCursor(14, 0);
      lcd.print(" ");
      lcd.setCursor(14, 1);
      lcd.print(" ");
      drawDig(m % 10, 11, 0);
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

byte last_day;
void drawDate()
{
  if (isMChanged) last_day = 0;
  if (last_day != t.day())
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
