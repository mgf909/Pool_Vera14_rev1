/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino Nano w/ ATmega328 & Optiboot, Platform=avr, Package=arduino
*/

#define __AVR_ATmega328p__
#define __AVR_ATmega328P__
#define ARDUINO 101
#define ARDUINO_MAIN
#define F_CPU 16000000L
#define __AVR__
#define __cplusplus
extern "C" void __cxa_pure_virtual() {;}

void setup(void);
boolean IsRemoteOnOff();
void loop(void);
void startAcidPump();
void stopAcidPump();
void startSolarPump();
void stopSolarPump();
void startFilterPump();
void stopFilterPump();
void doBeep();
void digitalClockDisplay();
void print2digits(int number);
void printRoofTemp(float tempA);
void printPoolTemp(float tempA);
void printStatus(int statusCode);
void printPHInfo(float ph);
void printAcidtime(int acidTime);
void printPtime(int pTime);
void SetTime();
void receiveTime(unsigned long VeraTime);
void sendData( float roofTemp,float poolTemp, float pH, int pTime,int acidTime);
void incomingMessage(const MyMessage &message);
boolean IsItWinter();
boolean IsOffPeak();
float measurePHVolts();
float getTempAdjusted4();
float measurePH();
float doPHTempCompensation(float PH, float temp);

#include "D:\arduino\arduino-1.0.5- Mysensors1.4\hardware\arduino\variants\eightanaloginputs\pins_arduino.h" 
#include "D:\arduino\arduino-1.0.5- Mysensors1.4\hardware\arduino\cores\arduino\arduino.h"
#include "C:\Users\greg\Google Drive\Arduino\Pool_Vera14_rev1\Pool_Vera14_rev1.ino"
#include "C:\Users\greg\Google Drive\Arduino\Pool_Vera14_rev1\ControlFunctions.ino"
#include "C:\Users\greg\Google Drive\Arduino\Pool_Vera14_rev1\LCDScreens.ino"
#include "C:\Users\greg\Google Drive\Arduino\Pool_Vera14_rev1\SetClock.ino"
#include "C:\Users\greg\Google Drive\Arduino\Pool_Vera14_rev1\VeraData.ino"
#include "C:\Users\greg\Google Drive\Arduino\Pool_Vera14_rev1\WinterOffPeak.ino"
#include "C:\Users\greg\Google Drive\Arduino\Pool_Vera14_rev1\phStuff.ino"
