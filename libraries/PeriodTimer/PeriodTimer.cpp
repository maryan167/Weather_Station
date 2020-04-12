#include "PeriodTimer.h"
#include <Arduino.h> 

PeriodTimer::PeriodTimer(uint32_t interval) {
	_interval = interval;
}

boolean PeriodTimer::isReady() {
	if(status) {
		_timer = millis();
		status = false;
		return true;
	}
	if (millis() - _timer >= _interval) {
		_timer = millis();
		return true;
	} else return false;
}
