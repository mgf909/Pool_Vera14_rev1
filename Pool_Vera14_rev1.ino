#include <DS1307RTC.h> //for RTC
#include <OneWire.h> //for temp sensors
#include <DallasTemperature.h> //for temp sensors
#include <bv4208.h> //for LCD
#include <I2c_bv.h> //for LCD
#include <EEPROM.h> //for Radio
#include <MySensor.h>  //for MySensors function
#include <SPI.h> //for Radio
#include <Time.h> //for RTC
#include <Wire.h> //for LCD and RTC


/*
need to re-add the remote code
need to add timer from when the pump stops  until its allowed to start again
Interupt function for the pH tuning
- powersupply for the phshield
-big capacitor for the VCCin

- DONE Sync time on Sundays or if time not set. - Not completed/tested - DONE
- Get varibles from Vera and write to eprom - maxpooltemp, differential,acidruntime
- On bootup ( or via button )  - show vera info , script version etc.
- Remote switch - should toggle state rather than fixed.
	-also control each output independantly.
	-Switch on Light output
- Different Beep Types - Error - frequent beeping ( lost temp sensor)
- FIX: If Controller is unregistered from Vera, it wont startup....some blocking?
 -
*/

char sketch_name[] = "Pool Controller";
char sketch_ver[]  = "2014.9.15r1";


#define TESTBED 1
#define DEBUG 0
#define PHCAL 0

boolean metric = true; 

tmElements_t tm;
unsigned long veraTime =0L;

// Connections to Arduino
//SDA  = A4 
//SCL = A5
#define ONE_WIRE_BUS 4 //Pin 4 - Keep Pin2 free for radio
//Interupt pins are 2 and 3 - will need to use pin3 and swap/ rewire the beeper and remote pins.
const int phPin = A2;  // analog pH probe sensor
const int beepPin  = 3;  // Beeper 
const int solarpumpPin = 5;   // this was the opPin, now used for solarpump
const int acidpumpPin = 6; // This is the relay for the acid feeder ( this was the valve relay in v3 )
const int filterpumpPin  = 7;   // Relay for Filter Pump
const int chlorinatorPin = 8;  //chlorinator SSR - moved from 9 to 8
const int remotePin   = A3;  // remote control circuit. - MOVE THIS TO ANALOG PIN moved from 8


//Set LCD Address
BV4208 lcd(0x21);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature dallasSensors(&oneWire);

//set states and variables

#if TESTBED == 1
#define MAXPOOLTEMP 29
int differential = 2; // Set the differential to switch on the pumps. In Production this should be about 15, on testbed 0
int acidpumpOnTime = 5; //how many minutes on the hour to run the acid feeder for.
// Set Temp Sensor addresses
DeviceAddress roofThermometer = { 
  0x28, 0x61, 0x17, 0x1B, 0x04, 0x00, 0x00, 0xF5 };
DeviceAddress poolThermometer = { 
  0x28, 0xEB, 0x68, 0xE3, 0x02, 0x00, 0x00, 0x6E };

#endif

#if TESTBED == 0
#define MAXPOOLTEMP 27
int differential = 1; // Set the differential to switch on the pumps. In Production this should be about 15, on testbed 0
int acidpumpOnTime = 05; //how many minutes on the hour to run the acid feeder for.
// Set Temp Sensor addresses
DeviceAddress roofThermometer = { 
  0x28, 0x76, 0x30, 0xE3, 0x02, 0x00, 0x00, 0x3E };
DeviceAddress poolThermometer = { 
  0x28, 0xBE, 0xFF, 0x0C, 0x02, 0x00, 0x00, 0x9E };

#endif


#define SUMMERMAXPUMPRUNTIME 28800  // 28800 seconds = 8 hours, 36000 = 10 HOURS
#define WINTERMAXPUMPRUNTIME 14400 // 4 Hours only during winter. - Winter could also be enabled when pooltemp drops to 18deg 
int MAXPUMPRUNTIME;

boolean filterpumpRunning = false;
boolean solarpumpRunning = false;
boolean veraRemote = false;
boolean timeReceived = false;


int VeraCounter = 0; //counter for Vera update period

int currentMonth;
int currentHour;
int currentMinute;
float idealpH = 7.30; // set the ideal pH level for the pool
float acidpumpRunTime = 0;
int SolarStartcount = 0;
int SolarStartDelay = 6; // delay for solar pump startup
int remoteMode = 1;
int ItsWinter = 0;
int veraSwitchstate;
unsigned long yesterdaysPRT;
unsigned long pumpRunTime = 0;
unsigned long loopTime;
unsigned long loopstartmillis = 0;
unsigned long lastPumpstart = 0;
unsigned long lastPumpstop = -40000; // give it 40 secs for initial starting.
unsigned long huristicdelay = 40000; //40 sec delay between pump restarts

int pTime ;
int bTime = 0;
int beepState = LOW;
long  pBeepMillis = 0;

//pH Settings - change when calibrating
float volt4 = 3.341;
float volt7 = 2.304;
float calibrationTempC = 31.1;
float avgMeasuredPH= 0;


// Array of Status codes 0-9
char* strStatus[]= {
  "OFFPEAK","MAX PRT","POOLMAXTEMP","HEATING", "HEURISTIC", "NO SUN", "READY", "ERR-SHIT","REMOTE ON","WINTER"};
int statuscode; // a number for each of the above 0-9

#define Roof_CHILD_ID 1		// Id of the sensor child
#define Pool_CHILD_ID 2		// Id of the sensor child
#define pH_CHILD_ID 3		// Id of the sensor child
#define prt_CHILD_ID 4		//id of the PumpRunTime child
#define hcl_CHILD_ID 5		//id of the Acid Pump Runtime child
#define Pump_CHILD_ID 6		//id of the Pump Overide switch child

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 20000;  // 20s delay between updates to Vera


//Initialise the MySensor library.
MySensor gw;

// Initialize temperature message
MyMessage roofTempmsg(Roof_CHILD_ID, V_TEMP);
MyMessage poolTempmsg(Pool_CHILD_ID, V_TEMP);
MyMessage pHValuemsg(pH_CHILD_ID, V_TEMP);
MyMessage prtValuemsg(prt_CHILD_ID, V_TEMP);
MyMessage hclValuemsg(hcl_CHILD_ID, V_TEMP);
MyMessage PumpCntrlmsg(Pump_CHILD_ID, V_LIGHT);





void setup(void)
{

// Startup and initialize MySensors library. Set callback for incoming messages.

  gw.begin(incomingMessage,5); //ID is 5


// Send the Sketch Version Information to the Gateway
gw.sendSketchInfo(sketch_name, sketch_ver);


setSyncProvider(RTC.get);
//setSyncInterval(3600);  // get time from gw every hour

// Register all sensors to gw (they will be created as child devices)
	gw.present(Roof_CHILD_ID, S_TEMP);
	gw.present(Pool_CHILD_ID, S_TEMP);
	gw.present(pH_CHILD_ID, S_TEMP);
	gw.present(prt_CHILD_ID, S_TEMP);
	gw.present(hcl_CHILD_ID, S_TEMP);
	gw.present(Pump_CHILD_ID, S_LIGHT);



  //Setup LCD
	lcd.bl(0); // turn BL on
	lcd.cls(); // clear screen

  //Set Pin Modes
	pinMode (acidpumpPin, OUTPUT);
	pinMode (solarpumpPin, OUTPUT);
	pinMode (filterpumpPin, OUTPUT);
	pinMode (chlorinatorPin, OUTPUT); 
	pinMode (beepPin, OUTPUT); 
	pinMode (remotePin, INPUT_PULLUP);


  // Start up the Dallas Temp library and set the resolution to 9 bit
  dallasSensors.begin();
  dallasSensors.setResolution(poolThermometer, 12);
  dallasSensors.setResolution(roofThermometer, 12);

setSyncProvider(RTC.get); 
//setSyncInterval(3600);  // get time from gw every hour


//End of setup
}





//change this to use a pushbutton so i can just toggle the state!
boolean IsRemoteOnOff() {  
  if ( digitalRead(remotePin) == HIGH || veraRemote == TRUE  ) {
   return true;
  }
  else {
    return false;   
  }
}



void loop(void)
{ 
   
  loopstartmillis = millis(); //start a counter for the pump runtimes  

  VeraCounter ++; //this counter for requesting data only periodically
    
  gw.process();
  
  
 if (VeraCounter == 30){
   gw.requestTime(receiveTime);  //Ask Vera for the time

//   gw.getStatus(Pool_CHILD_ID, V_VAR1); //does getStatus hang the sketch if vera offline? - use request status instead..
//   TempStatus(gw.getMessage());
//    gw.getStatus(CHILD_ID_TEMP, V_VAR2);
//    LowStatus(gw.getMessage());

//    gw.getStatus(CHILD_ID_TEMP, V_VAR3);
//    HighStatus(gw.getMessage());

//    gw.getStatus(CHILD_ID_TEMP, V_VAR4);
//    WeatherStatus(gw.getMessage());
//    Serial.println(TodayWeather);
    VeraCounter = 0;
  }


//Sync the time with Vera - typically for DST changes.
 //if ((tm.Wday = 1) && (tm.Hour = 23) | (hour() != tm.Hour)) {
 if (currentHour != tm.Hour) {
  Serial.println(" Syncing time with Vera");
  SetTime();
 }





//Ensure Backlight is turned on!
 lcd.bl(0); // turn BL on

  // Get temperature values
  dallasSensors.requestTemperatures();
  float poolTemp = dallasSensors.getTempC(poolThermometer);
  float roofTemp = dallasSensors.getTempC(roofThermometer);


//Send data to Vera - need to add pump status, times,  I need a device type which shows red/green and value. Then can add status.
 if(millis() - lastConnectionTime > postingInterval) {
    Serial.println("Uploading data to Vera");
    sendData(roofTemp,poolTemp,avgMeasuredPH,pumpRunTime,acidpumpRunTime);
   }
 
 




  //Print Data to LCD
  digitalClockDisplay();
  printRoofTemp(roofTemp);
  printPoolTemp(poolTemp);
  printStatus(statuscode);
  printAcidtime(acidpumpRunTime);
  printPtime(pumpRunTime);
  printPHInfo(avgMeasuredPH);

/*
// Check for messages from Vera
if (gw.messageAvailable()) {
    // ot new messsage from gw
    message_s message = gw.getMessage(); 
    setRelayStatus(message);
  }
*/


//remote control section
   if(DEBUG){
      Serial.print(F("Remote State:"));
      Serial.println(remoteMode);
	   }
     

  //if switch in on then Isremoteonoff=true
  //if remote is on toggle a state remotemode
    if (IsRemoteOnOff()) {
   // Serial.println("Remote Mode");
    if (remoteMode == 1) {
    }
    else if (filterpumpRunning == true)  {
      Serial.println("Filter Pump is running... will stop it");
      stopFilterPump();
      stopSolarPump();
      remoteMode = 1;
    }
    else if (filterpumpRunning == false)  {
  //  Serial.println("Filter Pump is Stopped... will Start it");
      startFilterPump();
      remoteMode = 1;
    }

    doBeep();  //beep beeper to indicate remote is active
    
  }  
  else {
  //  Serial.println("Remote is Off");
    remoteMode = 0 ;
  }



  //Check if its Winter or Summer
  if (IsItWinter()) {
    ItsWinter = 1;
    MAXPUMPRUNTIME = WINTERMAXPUMPRUNTIME;
   }
  else {
    ItsWinter = 0;
    MAXPUMPRUNTIME = SUMMERMAXPUMPRUNTIME;
   }


  // If its Off Peak then run the pump for the remaining required time
  if (IsOffPeak()) {
    startFilterPump();
    statuscode = 0; //Off Peak
  }

  // we want to reset the pump runtime counter at 7 am everyday
  // we will dedicate the entire 0 minute of 7 am to the reset to ensure we catch it.
  if ((currentHour == 7) && (currentMinute == 0)) {
    yesterdaysPRT = pumpRunTime;
    pumpRunTime = 0;
    //    lcd.clear();
    delay(5000);    // rethink about this delay, maybe longer?
    return;         // don't know if this will break out of the loop()
  }


//log amount of acid delivered....


  //Begin pH stuff
  int x;
  int sampleSize = 500;
  float avgRoomTempMeasuredPH =0;
  float avgTemp = 0;
  float avgPHVolts =0;
  float avgVoltsPerPH =0;
  float phTemp = 0;

  float tempAdjusted4 = getTempAdjusted4();

  for(x=0;x< sampleSize;x++)
  {
    float measuredPH = measurePH();
    float phTemp = poolTemp;
    float roomTempPH = doPHTempCompensation(measuredPH, phTemp);
    float phVolt = measurePHVolts();

    avgMeasuredPH += measuredPH;
    avgRoomTempMeasuredPH += roomTempPH;
    avgTemp += phTemp;
    avgPHVolts += phVolt;
  }

  avgMeasuredPH /= sampleSize;
  avgRoomTempMeasuredPH /= sampleSize;
  avgTemp /= sampleSize;
  avgPHVolts /= sampleSize;

#if PHCAL == 1 
  Serial.print(F(" measuredPH-"));
  Serial.print(avgMeasuredPH,4);
  Serial.print(F(" roomTempMeasuredPH-"));
  Serial.print(avgRoomTempMeasuredPH,4);
  Serial.print(F(" tempC-"));
  Serial.print(avgTemp,4);
  Serial.print(F(" phVolts-"));
  Serial.print(avgPHVolts,4);
  Serial.print(F(" 7CalVolts-"));
  Serial.print(volt7,4);
  Serial.print(F(" 4CalVolts-"));
  Serial.print(volt4,4);    
  Serial.print(F(" 4CalTempAdjusted-"));
  Serial.println(tempAdjusted4,4);
#endif    


  // *****Begin pH monitoring section ****
  //get the ph and start adding if needed. 
  //Run the acid pump for a count of X - reset the count every 30mins
  //need to add a timer so only runs for 5mins or so each hour.

  if ((avgMeasuredPH > idealpH) && ( currentMinute < acidpumpOnTime )) { //Turn on acid pump if needed and first few minutes of the hour.
    startAcidPump();
  }	else {
    stopAcidPump();	//Turn off the acid pump if the ph is OK or not the right time.
  }
  //perhaps fire an alarm if PH really OUT!




// *****Begin Heating and Pump control ****
  if ((pumpRunTime >= MAXPUMPRUNTIME) && (remoteMode == 0)){
    statuscode = 1; //MaxPumpTime
    stopFilterPump();
    stopSolarPump(); //this may not be necessary but better to leave here in case pool is heating in OP hours!

    return; //not sure what this return does???
  }


  else if ((IsOffPeak()) && (remoteMode == 0))  { // If its Off Peak then run the pump for the remaining required time
    startFilterPump();
    statuscode = 0; //Off Peak
  }

  else if ((poolTemp >= MAXPOOLTEMP) && (remoteMode == 0)) { // This condition is when the pool has reached MAX temperature. No pumps running during peak hours
  if (lastPumpstart + huristicdelay < millis()) {   //dont stop the pumps if they only just started..await the huristic delay period.
		stopSolarPump();
		stopFilterPump();
     statuscode = 2; //Pool Max Temp
	 } 
  }

  else if ((roofTemp > poolTemp + differential) && (poolTemp < MAXPOOLTEMP) && (ItsWinter == 0) && (remoteMode == 0)) { // This condition is when the pool will be heated - both pumps running.  
    if (lastPumpstop + huristicdelay < millis()) {  //dont start the pumps if they have only just stopped. Await the delay.
		startFilterPump();
		startSolarPump();
      statuscode = 3; // HEATING
	  }
  }

  
  else if ((roofTemp <= poolTemp + differential) && (remoteMode == 0)) { //This condition is for when there is not enough solar heat to switch on the pumps.  
    if (pumpRunTime == 0 ){ //bodge to set initial statuscode
		statuscode = 6; // "READY"
    }
    
  else if (remoteMode == 0)  {
      stopFilterPump();
      stopSolarPump();
      statuscode = 5; //NO SUN
    } 
  }
	
  else if (remoteMode == 1){
	statuscode = 8; //Remote ON
}

  else if (ItsWinter == 1) {
    statuscode = 9; // WINTER
  }
  else {
    // if we get to here for whatever reason, we should just stop the pumps
    //this will occur if poolTemp>maxpooltemp and roofTemp>pool+differential
    //    stopFilterPump();
    //    stopSolarPump();
    //    statuscode = 7; // ERR-SHIT
    statuscode = 4; // heuristic
  }





  //Generate the Pump Runtime for how long the pump needs to run each day.
  if (filterpumpRunning == true){
		loopTime = (millis() - loopstartmillis)/1000; //calculate how long the loop took to run
		pumpRunTime =(pumpRunTime+loopTime); //calculate the new pumpRunTime value
  }
 
}  //end of sketch














