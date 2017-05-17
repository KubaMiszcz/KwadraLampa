/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the Uno and
  Leonardo, it is attached to digital pin 13. If you're unsure what
  pin the on-board LED is connected to on your Arduino model, check
  the documentation at http://arduino.cc

  This example code is in the public domain.

  modified 8 May 2014
  by Scott Fitzgerald
 */

 /*
 jak dzial starts with cyz licz od 0
 i czy aa.startswith("aa") jest ok?
 */


 // the setup function runs once when you press reset or power the board
#include <SoftwareSerial.h>
#include "Lamp.h"
#include "SharedObjects.h"
#include <EEPROM.h>

//stale
const uint8_t numLamps = 2;
enum Modes { Manual = 0, Blututu };
const unsigned long int SecondInMillis = 1000;
const unsigned int CycleDelay = 10;

//komunikaty IN
//const String MsgSetClock = "HM:";
//const String MsgGetClock = "HM:?";

//komunikaty OUT


//pinout
//Lampa 1 - gorna 20W
const uint8_t inLamp1BrightnessPot = A2;
const uint8_t outLamp1OutPWM = 6;
LampClass Lamp1("L1", inLamp1BrightnessPot, outLamp1OutPWM);

//!!!!!!!!!! 
//nazwy musza byc L1 L2... bo tak jest potem w komunikatach na sztywno ustawione inna nazwa i komunikat nie zadziala
//!!!!!!!!!!!!!!

//Lampa 2 - gorna 4W
const uint8_t inLamp2BrightnessPot = A1;
const uint8_t outLamp2OutPWM = 5;
LampClass Lamp2("L2", inLamp2BrightnessPot, outLamp2OutPWM);

//software serial
SoftwareSerial mySerial(13, 12); // RX, TX

LampClass Lamps[] = { Lamp1,Lamp2 }; //3 zeby nie zaczynac od indeksu 1-L1, 2-L2


//zmienne
String incomingMesg = "";
int readings[] = { 0,0,0 };
unsigned long int NextTick;
//unsigned int Hours = 0, Minutes = 0, Seconds = 0;
Time currentTime;
//SharedObjectsClass SharedObjects;

void setup() {
	Serial.begin(9600);
	Serial.println("hello serial");
	mySerial.begin(9600);
	mySerial.println("hello Myserial v2");
	pinMode(inLamp1BrightnessPot, INPUT);
	pinMode(outLamp1OutPWM, OUTPUT);
	pinMode(inLamp2BrightnessPot, INPUT);
	pinMode(outLamp2OutPWM, OUTPUT);
	NextTick = millis() + SecondInMillis;//+1 second

	for (int i = 0; i < numLamps; i++)
	{
		UpdateReadings(i, Lamps[i].inBrightnessPot);
		Serial.print("i= " + String(i));
		Serial.print(" name= " + Lamps[i].Name);
		Serial.print(" pin= " + String(Lamps[i].inBrightnessPot));
		Serial.println(" read= " + String(readings[i]));
		Lamps[i].SetBrightness(readings[i]);
		Lamps[i].IsTurnedON = false;
		Lamps[i].prevBTBrightness = Lamps[i].currentBrightness;
		Lamps[i].prevManualBrightness = Lamps[i].currentBrightness;
	}
}

//void loop2() {
//	int read = analogRead(Lamp1.inBrightnessPot);//0-1023
//	Serial.print(read); Serial.print(" ");
//	int curManBrightness = map(read, 0, 1023, 0, 255);//0-255
//	Serial.print(curManBrightness); Serial.print(" ");
//	Lamp1.SetBrightness(curManBrightness);
//	Serial.print(Lamp1.currentBrightness); Serial.print(" \n");
//}


// the loop function runs over and over again forever
void loop() {
	delay(CycleDelay);
	//########## dodaj pobieranie satusu zmiennych zeby poustwaiac kontrolki w apce przy polaczeniu 
	//przeyslanie timera stanow alampek jasnosci itp
	String str = ReceiveMesg();	//odebranie wiadomosci z nadajnika 
	SendMesg();
	UpdateClock();
	//ReceiveMesg222();
	//UpdateClockv2(&Seconds);

	for (uint8_t i = 0; i < numLamps; i++)
	{
		//##############################################
		//zzaczynamy w trybie manual
		//##############################################
#pragma region MANUAL currentMode
		if (Lamps[i].currentMode == Modes(Manual))	//MANUAL
		{	//przejsice w blututu
			if (str.startsWith(Lamps[i].Name))
			{
				Lamps[i].currentMode = Modes(Blututu);
				Lamps[i].prevMode = Modes(Manual);

				Serial.println("curmode " + Lamps[i].Name + " =man>>BT"
					+ String(Lamps[i].currentMode) + " "
					+ String(Lamps[i].currentBrightness));

				//zapamietanie ostatneigo brighness w MAN
				UpdateReadings(i, Lamps[i].inBrightnessPot);
				Lamps[i].prevManualBrightness = readings[i];
			}
			else {	//zostajemy w manual
				UpdateReadings(i, Lamps[i].inBrightnessPot);
				//Serial.print("man analog read " + String(read));
				//Serial.println(" man mapped analog readings[i] " + String(readings[i]));
				Lamps[i].SetBrightness(readings[i]);
				//if (Lamps[i].currentBrightness < 5)
				//{
				//	//mySerial.println("::L1:PWM" + String(Lamp1.currentBrightness) + "\n");
				//}
			}
		}
#pragma endregion

		//##############################################
		//a teraz jesli tryb BT	//blutut
		//##############################################
#pragma region Blutut Mode
		if (Lamps[i].currentMode == Modes(Blututu))	//BLUTUT
		{
			Lamps[i].ActionOnTimer(); //tylko w btmode, w manual nie ma timera
			//czy przejsc w man mode
			UpdateReadings(i, Lamps[i].inBrightnessPot);
			if (
				(readings[i] > (Lamps[i].prevManualBrightness + 5))		//filtre bo ADC plywa moze dodoac jakies 100nF?
				||
				(readings[i] < (Lamps[i].prevManualBrightness - 5)))
			{
				Lamps[i].currentMode = Modes(Manual);
				Lamps[i].prevMode = Modes(Blututu);
				Lamps[i].SetBrightness(readings[i]);
				Serial.println("curmode= " + Lamps[i].Name + " BT>>MAN"
					+ String(Lamps[i].currentMode) + " "
					+ String(Lamps[i].currentBrightness));
			}
			else if (str.startsWith(Lamps[i].Name)) {
				Serial.println(str);
				Lamps[i].ActionOnMessage(str);//a co z tym stringiem zrobic robi metoda w klasie
			}
		}
#pragma endregion 
	}
	//##########KONIEC PRZELATYWANIA PO TABLICY LAMP


	//#############ZAPYTANIA DO STEROWNIKA
	//Przy przesylania pamietaj zeby dlugosc komunikatu byla 11 znakow
	//dodaj te :: na poczatku, ale juz do rozpoznawanie jest bez tych ::
	//###### ZAPYTANIA LUB USTAWIANIE PARAMETROW STEROWNIKA
	if (str.startsWith("HM:")) {		//HM:1234xx
		if (str.startsWith("HM:?")) {		//ktora godizna
			mySerial.println("Clock: " + String(currentTime.H) + ":" + String(currentTime.M) + ":" + String(currentTime.S));
			Serial.println("Clock send: " + String(currentTime.H) + ":" + String(currentTime.M) + ":" + String(currentTime.S));
		}
		else	//ustawianie zegara
		{
			currentTime.H = str.substring(3, 5).toInt();
			currentTime.M = str.substring(5, 7).toInt();//
			SharedObjects.SetTime(currentTime.H, currentTime.M, currentTime.S);
			Serial.println("Clock Updated: " + String(currentTime.H) + ":" + String(currentTime.M) + ":" + String(currentTime.S));
		}
	}

}

String ReceiveMesg() {
	String str = "";
	if (mySerial.available() > 0) {
		str = mySerial.readString();
		str.trim();
		Serial.println("Received>> " + str);
		if (str.startsWith("::")) {
			if (str.length()>11) {
				str = str.substring(str.length() - 9); //9 osttanich znakow, juz bez "::"
			}
			else {
				str =str+"xxxxxxxxxxx";				
				str = str.substring(2, 11);
			}
		}
		else str = "";
	}
	else str = "";
	return str;
}

void SendMesg() {
	if (SharedObjects.MessageToSend != "") {
		mySerial.println(SharedObjects.MessageToSend);
		SharedObjects.MessageToSend = "";
	}
}

//void ReceiveMesg222() {
//	String str = "";
//	if (Serial.available() > 0) {
//		str = Serial.readString();
//		Serial.println("Received222>> " + str);
//	}
//}


void UpdateReadings(uint8_t i, uint8_t BrightnessPotentiometer) {
	readings[i] = analogRead(BrightnessPotentiometer);//0-1023
	readings[i] = map(readings[i], 0, 1023, 0, 255);//0-255
}

void UpdateClock() {
	if (millis() > NextTick)
	{
		currentTime.S += 1;
		if (currentTime.S >= 60)
		{
			currentTime.S = 0;
			currentTime.M += 1;
			Serial.println("CLK: " + String(currentTime.H) + ":" + String(currentTime.M) + ":" + String(currentTime.S));
			if (currentTime.M >= 60)
			{
				currentTime.M = 0;
				currentTime.H += 1;
				if (currentTime.H >= 24) {
					currentTime.H = 0;
				}
			}
		}
		SharedObjects.SetTime(currentTime.H, currentTime.M, currentTime.S);
		//Serial.println("CLK: " + String(currentTime.H) + ":" + String(currentTime.M) + ":" + String(currentTime.S));
		NextTick += SecondInMillis;
	}
}

//void UpdateClockv2(unsigned int* ss) {
//	if (millis() > NextTick)
//	{
//		*ss = *ss + 1;
//
//		if (*ss >= 60)
//		{
//			*ss = 0;
//		}
//		String str = String(*ss);
//		Serial.println("CLK: " + str);
//		NextTick += SecondInMillis;
//	}
//}