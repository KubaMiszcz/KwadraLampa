// SharedObjects.h

#ifndef _SHAREDOBJECTS_h
#define _SHAREDOBJECTS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
struct Time {
	uint8_t H; uint8_t M; uint8_t S;
};


class SharedObjectsClass
{
private:
	static Time currentTime;

 public:
	void init();
	static String MessageToSend;

	int GetHours() {
		return currentTime.H;
	}
	void SetHours(uint8_t val) {
		if (val<0 || val>23)currentTime.H=0;
		else currentTime.H = val;
	}

	int GetMinutes() {
		return currentTime.M;
	}
	void SetMinutes(uint8_t val) {
		if (val<0 || val>59)currentTime.M = 0;
		else currentTime.M = val;
	}
	int GetSeconds() {
		return currentTime.S;
	}
	void SetSeconds(uint8_t val) {
		if (val<0 || val>59)currentTime.S = 0;
		else currentTime.S = val;
	}

	void SetTime(uint8_t h, uint8_t m, uint8_t s) {
		currentTime.H = h;
		currentTime.M = m;
		currentTime.S = s;
	}
	Time GetTime() {
		return currentTime;
	}
	};

extern SharedObjectsClass SharedObjects;

#endif


