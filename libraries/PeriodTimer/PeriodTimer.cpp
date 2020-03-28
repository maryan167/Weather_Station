#include "PeriodTimer.h"
#include <Arduino.h> 

PeriodTimer::PeriodTimer(unsigned long interval) {
	_interval = interval;
}

boolean PeriodTimer::isReady() {
	if (short(millis() - _timer) >= 0) {
		_timer = millis() + _interval;
		return true;
	} else return false;
}
