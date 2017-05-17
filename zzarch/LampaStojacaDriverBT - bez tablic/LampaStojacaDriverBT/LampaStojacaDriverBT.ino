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
//pinout
int inLamp1BrightnessPot=A1;
int outLamp1OutPWM = 5;
int inLamp2BrightnessPot = A2;
int outLamp2OutPWM = 6;
SoftwareSerial mySerial(13, 12); // RX, TX

//zmiene
String incomingMesg="";
enum modes { Manual = 0, Blututu };

//klasy
LampClass Lamp1(inLamp1BrightnessPot, outLamp1OutPWM);
LampClass Lamp2(inLamp2BrightnessPot, outLamp2OutPWM);

void setup() {
	Serial.begin(9600);
	Serial.println("hello serial");
	mySerial.begin(9600);
	Serial.println("hello Myserial");
    pinMode(inLamp1BrightnessPot, INPUT);
	pinMode(outLamp1OutPWM, OUTPUT);
	pinMode(inLamp2BrightnessPot, INPUT);
	pinMode(outLamp2OutPWM, OUTPUT);
	
	int read;
	read = analogRead(Lamp1.inBrightnessPot);//0-1023
	Serial.println(read);
	Lamp1.currentBrightness = map(read, 0, 1023, 0, 255);//0-255
	Lamp1.SetBrightness(Lamp1.currentBrightness);
	Lamp1.IsTurnedON = false;

}

void loop2() {
	int read = analogRead(Lamp1.inBrightnessPot);//0-1023
	Serial.print(read); Serial.print(" ");
	int curManBrightness = map(read, 0, 1023, 0, 255);//0-255
	Serial.print(curManBrightness); Serial.print(" ");
	Lamp1.SetBrightness(curManBrightness);
	Serial.print(Lamp1.currentBrightness); Serial.print(" \n");
}


// the loop function runs over and over again forever
void loop() {
	delay(50);
	
	//##############################################
	//wartosci current
	//##############################################
	String str = ReceiveMesg();
	int read;

	//##############################################
	//zzaczynamy w trybie manual
	//##############################################
#pragma region MANUAL mode
	if (Lamp1.mode==modes(Manual))	//MANUAL
	{
		if (str.startsWith("L1"))
		{
			Lamp1.mode = modes(Blututu);
			Lamp1.prevMode = modes(Manual);

			Serial.println("curmode=man>>BT" + String(Lamp1.mode)+" " + String(Lamp1.currentBrightness));
			mySerial.println("curmode=man>>BT" + String(Lamp1.mode) + " " + String(Lamp1.currentBrightness));
			
			//zapamietanie ostatneigo brighness w MAN
			read = analogRead(Lamp1.inBrightnessPot);
			read = map(read, 0, 1023, 0, 255);
			Lamp1.prevManBrightness=read;
		}
		else {
				read = analogRead(Lamp1.inBrightnessPot);
				//Serial.print("man analog read " + String(read));
				read = map(read, 0, 1023, 0, 255);
				//Serial.println(" man mapped analog read " + String(read));
				Lamp1.SetBrightness(read);
				if (Lamp1.currentBrightness<5)
				{
					//mySerial.println("::L1:PWM" + String(Lamp1.currentBrightness) + "\n");
				}

		}
	}
#pragma endregion

	//##############################################
	//a teraz jesli tryb BT	//blutut
	//##############################################
#pragma region Blutut Mode
	if (Lamp1.mode == modes(Blututu))	//BLUTUT
	{
		//czy przejsc w man mode
		int read = analogRead(Lamp1.inBrightnessPot);
		read = map(read, 0, 1023, 0, 255);
		if ( 
			(read > (Lamp1.prevManBrightness+5))
			|| 
			(read < (Lamp1.prevManBrightness-5)) )
		{
			Lamp1.mode = modes(Manual);
			Lamp1.prevMode = modes(Blututu);
			Lamp1.SetBrightness(read);
			Serial.println("curmode=BT>>MAN" + String(Lamp1.mode)+ " "+String(Lamp1.currentBrightness));
			mySerial.println("curmode=BT>>MAN" + String(Lamp1.mode) + " " + String(Lamp1.currentBrightness));
		}
		else
		{
			if (str.startsWith("L1")) {
				Lamp1.SendMesg(str);//a co z tym stringiem zrobic robi metoda w klasie
			}
		}
	}

#pragma endregion

	//##############################################
	//wart prev
	//##############################################
}

String ReceiveMesg() {
	String str = ""; 
	if (mySerial.available() > 0) {
		str = mySerial.readString();
		mySerial.println("echo>>"+str);
		if (str.startsWith("::")) {
			str = str.substring(str.length()-9); //9 osttanich znakow
			Serial.println(str);
			mySerial.println("echo>>" + str);
		}
		else str = "";
	}
	else str = "";
	return str;
}
