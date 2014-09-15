//control functions
void startAcidPump() {
	// this should only run if filter pump is running
	if (filterpumpRunning == true) {
		//start Acid Pump;
		digitalWrite (acidpumpPin, HIGH);
		//Add to Acid Runtime
		acidpumpRunTime =(acidpumpRunTime+loopTime);
		Serial.println("Pumping acid");
		Serial.println(acidpumpRunTime);
	}

	// dont want this as acid will run continuously if offpeak hours.
	if (filterpumpRunning == false) {
		//stop Acid Pump;
		digitalWrite (acidpumpPin, LOW);
		Serial.println("Filter Pump is NOT running - cannot pump acid");
	}
}

void stopAcidPump() {
	digitalWrite (acidpumpPin, LOW);
}

void startSolarPump() {
	// Need a delay here so as to allow the filter pump to get the wather flowing before starting the solar pump
	SolarStartcount++;
	if (SolarStartcount >= SolarStartDelay) {
		if (solarpumpRunning == false) {
			//start Solar Pump;
			digitalWrite (solarpumpPin, HIGH);
			solarpumpRunning = true;
		}
	}
}

void stopSolarPump() {
	if (solarpumpRunning == true) {
		// stop pump
		digitalWrite(solarpumpPin, LOW);
		//Reset the status and counters
		solarpumpRunning = false;
		SolarStartcount = 0;
	}
}

void startFilterPump() {
	if (filterpumpRunning == false) {
		// startfilterpump;
		lastPumpstart = millis();
		//gw.sendVariable(Pump_CHILD_ID, V_LIGHT, 1); // Send start to Vera
		digitalWrite(filterpumpPin, HIGH);
		digitalWrite(chlorinatorPin, HIGH);
		filterpumpRunning = true;
	}
}

void stopFilterPump() {
	if (filterpumpRunning == true) {
		// stop pump
		lastPumpstop = millis();
		//gw.sendVariable(Pump_CHILD_ID, V_LIGHT, 0); // send Stop to vera
		digitalWrite(filterpumpPin, LOW);
		digitalWrite(chlorinatorPin, LOW);
		filterpumpRunning = false;

	}
}

void doBeep() {
	//Serial.print(bTime);
	if (bTime == 20 ) { //this sets the time between beeps
		digitalWrite(beepPin, HIGH);
		delay(400);
		digitalWrite(beepPin, LOW);
		bTime = 0 ;
	}
	else
	bTime = bTime++ ;
}
