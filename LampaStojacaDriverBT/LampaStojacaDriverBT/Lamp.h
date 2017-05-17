// Lamp.h

#ifndef _LAMP_h
#define _LAMP_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#include "SharedObjects.h"

#else
#include "WProgram.h"
#endif

class LampClass
{
public:
	String Name;
	uint8_t inBrightnessPot;
	uint8_t outPWMPin;
	uint8_t currentBrightness;//0-255
	uint8_t currentMode;	//enum "BT" or "Man"
	uint8_t prevMode;
	bool IsTurnedON;
	uint8_t prevBTBrightness;//do sprawdzania czy zmienic mode
	uint8_t prevManualBrightness;//do sprawdzania czy zmienic mode
	bool IsTimerOn;
	bool IsClockOn;
	bool IsDimmerOn;
private:
	enum Modes { Manual = 0, Blututu };
	unsigned long int timerIntervalInMilliseconds;
	unsigned long int TimerStopTime;
	unsigned long int TimerStep=3000;//3sek
	unsigned long int TimerNextStep;
	unsigned long int DimmerStep;
	unsigned long int DimmerNextStep;

public:
	void init();
	LampClass() {};
	LampClass(String name, uint8_t potPin, uint8_t PWMpin) {
		Name = name;
		inBrightnessPot = potPin;
		outPWMPin = PWMpin;
		prevManualBrightness = map(analogRead(potPin), 0, 1023, 0, 255);//0-255
		currentBrightness = prevManualBrightness;
		prevBTBrightness = currentBrightness;
		currentMode = Modes(Manual);
		prevMode = currentMode;
		IsTurnedON = false;
		//prevBTBrightness;//do sprawdzania czy zmienic mode
		//prevManBrightness;//do sprawdzania czy zmienic mode
		timerIntervalInMilliseconds = 0;
		IsTimerOn = false;
		IsDimmerOn = false;
	}

	//0-255

	void SetBrightness(int val) {
		if (val < 0) {
			val = 0;
		}
		if (val > 255) {
			val = 255;
		}
		currentBrightness = val;
		float factor = val / 255.;
		int brightnessToSet = (uint8_t)(val*factor*factor*factor); //doswiadczalnie zeby jasnosc rosla w miare liniowo
		analogWrite(outPWMPin, brightnessToSet);
		//Serial.println("curbright "+Name+" = "+String(currentBrightness));
	}

	void ActionOnMessage(String str) { //reakcja na blutut
		str = str.substring(3);//usuniecie "L1:"
		Serial.println("str-3= " + str);
		
		//power on		//::L1:onxxxx
		if (str.startsWith("on")) {
			IsTurnedON = true;
			IsTimerOn = false;
			SetBrightness(prevBTBrightness);//ostatnia brightness a nie 255
			Serial.println("BT " + Name + " on prevBright=" + String(prevBTBrightness) + " on curBright=" + String(currentBrightness));
		}

		//power off		//::L1:offxxx
		if (str.startsWith("off")) {
			IsTurnedON = false;
			IsTimerOn = false;
			prevBTBrightness = currentBrightness;
			SetBrightness(0);
			Serial.println("BT " + Name + " on prevBright=" + String(prevBTBrightness) + " on curBright=" + String(currentBrightness));
		}

		//set brightness	//::L1:PWM12x
		if (str.startsWith("PWM")) {
			IsTurnedON = true;
			str = str.substring(3);//usuniecie "PWM"
			str.replace("x", "");//usuniecie xx
			currentBrightness = str.toInt();//przeyslane jako 0-255 a nie 0-100
			SetBrightness(currentBrightness);
			Serial.println("BT " + Name + " setBright= " + String(currentBrightness));
		}

		//######## TIMER //::L1:TIM12x
		if (str.startsWith("TIM")) {//set timer
			IsTimerOn = true;
			prevBTBrightness = currentBrightness;
			str = str.substring(3);//usuniecie "TIM"
			str.replace("x", "");//usuniecie xx
			unsigned long int timerValueInMinutes = str.toInt();//0-999 minut
			Serial.println("BT " + Name + " tim= " + String(timerValueInMinutes) + "min");

			//#########timer w minutach tak jak trzeba do release
			timerIntervalInMilliseconds = timerValueInMinutes * 60 * 1000;
			//##############

			//minuty jako skenudy - to potem usun tylko do testow
			//timerIntervalInMilliseconds = timerIntervalInMilliseconds / 60;

			TimerStopTime = millis() + timerIntervalInMilliseconds;
			Serial.println("BT " + Name + " curmillis= " + String(millis()));
			//Serial.println("BT " + Name + " curInterval= " + String(timerIntervalInMilliseconds));
			Serial.println("BT " + Name + " timStop= " + String(TimerStopTime));

			if (IsDimmerOn)
			{
				DimmerStep = timerIntervalInMilliseconds / currentBrightness;
				DimmerNextStep = millis() + DimmerStep;
			}
		}
		//######### cancel TIMER
		if (str.startsWith("TIcncl")) {//set dimmmer
			IsTimerOn = false;
			Serial.println("BT " + Name + " TIMCancel");
		}

		//########### CLOCK	//::L1:CLhhmm
		if (str.startsWith("CL")) {//set Clock on/off
			//CLOCK to to samo co timer tylko tutaj zamiast recznei wyliczac czas timer
			//to tutaj licze podst godziny obecnej i nastawionej
			IsTimerOn = true;
			prevBTBrightness = currentBrightness;
			uint8_t CLKHours = str.substring(2, 4).toInt();//
			uint8_t CLKMinutes = (str.substring(4, 6)).toInt();//
			Serial.println("BT " + Name + " CLKtim= " + String(CLKHours) + ":" + String(CLKMinutes));

			//przeliczanie na timer
			//dorob ifa gdy przepelnienie czyli godzina ustawiona wczesniej niz obecan godzina
			unsigned long int curTimeInMinutes =
				SharedObjects.GetTime().H * 60
				+ SharedObjects.GetTime().M;

			unsigned long int setTimeInMinutes =
				CLKHours * 60
				+ CLKMinutes;

			//#########timer tak jak trzeba do release
			if (setTimeInMinutes > curTimeInMinutes)
			{//godizna nastawiona pozniej niz obecna
				timerIntervalInMilliseconds = (setTimeInMinutes - curTimeInMinutes) * 60 * 1000;
			}
			else
			{//godizna nastawiona wczensije niz obecna
				timerIntervalInMilliseconds = ((24 * 60) + (setTimeInMinutes - curTimeInMinutes)) * 60 * 1000;
			}

			//minuty jako skenudy - to potem usun tylko do testow
			//timerIntervalInMilliseconds = timerIntervalInMilliseconds / 60;

			TimerStopTime = millis() + timerIntervalInMilliseconds;
			TimerNextStep = millis() + TimerStep;
			Serial.println("BT " + Name + " curmillis= " + String(millis()));
			//Serial.println("BT " + Name + " CLKInterv= " + String(timerIntervalInMilliseconds));
			Serial.println("BT " + Name + " CLKStop= " + String(TimerStopTime));

			if (IsDimmerOn)
			{
				DimmerStep = timerIntervalInMilliseconds / currentBrightness;
				DimmerNextStep = millis() + DimmerStep;
			}
		}

		//######### cancel CLOCK
		if (str.startsWith("Clcncl")) {//set dimmmer
			IsTimerOn = false;
			Serial.println("BT " + Name + " CLKCancel");
		}

		//############# DIMMER
		if (str.startsWith("DIMon")) {//set dimmmer
			IsDimmerOn = true;
			Serial.println("BT " + Name + " DIMon");
		}
		if (str.startsWith("DIMoff")) {//set dimmmer
			IsDimmerOn = false;
			Serial.println("BT " + Name + " DIMoff");
		}


		//############zapytania ::L1:DIMoff
		if (str.startsWith("?")) {//stan lampy
			if (currentBrightness>0)
				str= ("::" + Name + ":" + "onxxxx");
			else
			{
				str = ("::" + Name + ":" + "offxxx");
			}
			SharedObjects.MessageToSend = str;
			Serial.println("Status send: "+ str);
		}

		if (str.startsWith("DIM?")) {//stan sciemniacza
			if (IsDimmerOn)
				str = ("::" + Name + ":" + "DIMonxxxx");
			else
			{
				str = ("::" + Name + ":" + "DIMoffxxx");
			}
			SharedObjects.MessageToSend = str;
			Serial.println("Status send: " + str);
		}
	}

	void ActionOnTimer() {
		if (IsTimerOn)
		{
			if (millis() > TimerNextStep) {
				SetBrightness(0);
				delay(500);
				SetBrightness(255);
				TimerNextStep = millis() + TimerStep;
			}

			if (millis() > TimerStopTime)
			{
				IsTimerOn = false;
				IsTurnedON = false;
				SetBrightness(0);
				Serial.println("BT " + Name + " TIMFires");
			}
			if (IsDimmerOn)
			{
				if (millis() > DimmerNextStep)
				{
					SetBrightness(currentBrightness - 1);
					DimmerNextStep += DimmerStep;
					Serial.println("BT " + Name + " DimFires " + String(DimmerNextStep) + " Bright " + String(currentBrightness));
				}
			}
		}
	}

	//void TurnOff() {
	//	IsTurnedON = false;
	//	if (currentMode==Modes(Blututu))
	//	{
	//		prevBTBrightness = currentBrightness;	
	//		Serial.println("BT " + Name + " off " + String(currentBrightness));
	//	}
	//	else if (currentMode == Modes(Blututu))
	//	{
	//		prevManualBrightness = currentBrightness;
	//		Serial.println("Man " + Name + " off " + String(currentBrightness));
	//	}
	//	SetBrightness(0);
	//}

};

extern LampClass Lamp;

#endif
