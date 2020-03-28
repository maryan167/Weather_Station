#ifndef PeriodTimer_h
#define PeriodTimer_h
#include <Arduino.h>

class PeriodTimer
{
  public:
	PeriodTimer(unsigned long interval);
	boolean isReady();
	
  private:
	unsigned long _timer = 0;
	unsigned long _interval = 0;
};

#endif
