#ifndef PeriodTimer_h
#define PeriodTimer_h
#include <Arduino.h>

class PeriodTimer
{
  public:
	PeriodTimer(uint32_t interval);
	boolean isReady();
	
  private:
	boolean status = true;
	uint32_t _timer = 0;
	uint32_t _interval = 0;
};

#endif
