// Lamp.h

#ifndef _LAMP_h
#define _LAMP_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class LampClass
{
public:
	int inBrightnessPot;
	int outPWMPin;
	int currentBrightness;//0-255
	int mode;	//enum "BT" or "Man"
	int prevMode;
	bool IsTurnedON;
	int prevBTBrightness;//do sprawdzania czy zmienic mode
	int prevManBrightness;//do sprawdzania czy zmienic mode
	unsigned long timerValue;
	bool IsTimerOn;


 protected:
	 enum modes { Manual = 0, Blututu };
	 
 public:
	void init();
	LampClass() {};
	LampClass(int potPin, int PWMpin){
		inBrightnessPot = potPin;
		outPWMPin = PWMpin;
		currentBrightness = 0;
		mode = modes(Manual);
		prevMode = mode;
		IsTurnedON = false;
		//prevBTBrightness;//do sprawdzania czy zmienic mode
		//prevManBrightness;//do sprawdzania czy zmienic mode
		timerValue = 0;
		IsTimerOn = false;
	}

	//0-255
	void SetBrightness(int val) {
		currentBrightness =val;
		analogWrite(outPWMPin, currentBrightness);
		//Serial.println("curbright "+String(currentBrightness));
	}

	void SendMesg(String str) { //reakcja na blutut
		str=str.substring(3);//usuniecie L1:
		//Serial.println("str-3= " + str);
		if (str.startsWith("on")) {//power on
			IsTurnedON = true;
			currentBrightness = prevBTBrightness;
			SetBrightness(currentBrightness);//ostatnia brightness a nie 255
			Serial.println("BT L1 on " + String(currentBrightness));
		}
		if (str.startsWith("off")) {//power off
			IsTurnedON = false;
			prevBTBrightness = currentBrightness;
			SetBrightness(0);
			Serial.println("BT L1 off " + String(currentBrightness));
		}
		if (str.startsWith("PWM")) {//set brightness
			if (IsTurnedON)
			{	
				str = str.substring(3);//usuniecie PWM
				str.replace("x", "");//usuniecie xx
				//Serial.println(str2);
				currentBrightness = str.toInt();//przeyslane jako 0-255 a nie 0-100
				SetBrightness(currentBrightness);
				Serial.println("BT L1 setbr " + String(currentBrightness));
			}
		}

		if (str.startsWith("TIM")) {//set timer
			if (IsTurnedON)
			{
				str = str.substring(3);//usuniecie TIM
				str.replace("x", "");//usuniecie xx
									 //Serial.println(str2);
				timerValue = str.toInt();//0-999 minut
				IsTimerOn = true;
				//tu skonczylem#############################
				//Serial.println("BT L1 tim= " + String(currentBrightness));
			}
		}
	}

};

extern LampClass Lamp;

#endif

