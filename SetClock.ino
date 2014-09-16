/*
void SetTime() {  //get the time from vera and set RTC
	Serial.println("Time needs to be synced");
	tm.Hour = hour();
	tm.Minute = minute();
	tm.Second = second();
	tm.Day = day();
	tm.Month = month();
	tm.Year = CalendarYrToTm(year());
	RTC.write(tm);
}


// This is called when a new time value was received
void receiveTime(unsigned long VeraTime) {
	RTC.set(VeraTime); // this sets the RTC to the time from Vera - which i do want periodically
	timeReceived = true;
}
*/


// This is called when a new time value was received
void receiveTime(unsigned long controllerTime) {
	// Ok, set incoming time
	Serial.print("Time value received: ");
	Serial.println(controllerTime);
	RTC.set(controllerTime); // this sets the RTC to the time from controller - which we do want periodically
	timeReceived = true;
}